/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/errno.h>
#include <dxgmx/kboot.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/heap.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/mregmap.h>
#include <dxgmx/mem/pagefault.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/pd.h>
#include <dxgmx/x86/pdpt.h>
#include <dxgmx/x86/pt.h>

#define KLOGF_PREFIX "mm: "

#define PS2PDPT(_ps) ((pdpt_t*)(_ps->data))

#define FOR_EACH_PTE_IN_RANGE(s, e, pt, pte)                                   \
    for (pte_t* pte = pte_from_vaddr(s, pt); pte; pte = NULL)                  \
        for (ptr _i = s; _i < e; _i += PAGESIZE, pte = pte_from_vaddr(_i, pt))

#define FOR_EACH_KPTE_IN_RANGE(s, e, pte)                                      \
    for (pte_t* pte = pte_from_vaddr_abs(s, g_pdpt); pte; pte = NULL)          \
        for (ptr _i = s; _i < e;                                               \
             _i += PAGESIZE, pte = pte_from_vaddr_abs(_i, g_pdpt))

static void mm_tlb_flush_single(ptr vaddr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

static pd_t* pd_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    ptr pd_base = pdpte_pd_paddr(&pdpt->entries[vaddr / GIB]);
    return pd_base ? (void*)mm_kpa2va(pd_base) : NULL;
}

static pde_t* pde_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    pd_t* pd = pd_from_vaddr_abs(vaddr, pdpt);
    return pd ? pde_from_vaddr(vaddr, pd) : NULL;
}

static pt_t* pt_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    pde_t* pde = pde_from_vaddr_abs(vaddr, pdpt);
    if (!pde)
        return NULL;

    ptr pt_base = pde_pt_paddr(pde);
    return pt_base ? (void*)mm_kpa2va(pt_base) : NULL;
}

static pte_t* pte_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    pt_t* pt = pt_from_vaddr_abs(vaddr, pdpt);
    return pt ? pte_from_vaddr(vaddr, pt) : NULL;
}

static void mm_set_pte_and_pde_flags(pte_t* pte, pde_t* pde, u16 flags)
{
    /* If a pde has higher privileges, we don't demote them, because some other
     * page tables might make use of those. */

    if (!pde->writable && (flags & PAGE_W))
        pde->writable = true;

    pte->writable = !!(flags & PAGE_W);

    if (pde->exec_disable && (flags & PAGE_X))
        pde->exec_disable = false;

    pte->exec_disable = !(flags & PAGE_X);

    /* User access is different since we should not have, for example two
     * pagetables with different user_acess values within the same pagedir
     * anyway. */
    pde->user_access = !!(flags & PAGE_USER);
    pte->user_access = !!(flags & PAGE_USER);
}

_INIT void mm_setup_paging_arch(PagingStruct* kps)
{
    /* Enable NXE bit. */
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);
    kps->data = (void*)mm_kpa2va(cpu_read_cr3());
}

ptr mm_va2pa_arch(ptr vaddr, const PagingStruct* ps)
{
    pte_t* pte = pte_from_vaddr_abs(vaddr, ps->data);
    return pte ? (ptr)pte_frame_paddr(pte) + vaddr % PAGESIZE : (ptr)NULL;
}

int mm_init_paging_struct_arch(PagingStruct* ps)
{
    ps->data = kmalloc_aligned(sizeof(pdpt_t), PAGESIZE);
    if (!ps->data)
    {
        pagingstruct_destroy(ps);
        return -ENOMEM;
    }

    memset(ps->data, 0, sizeof(pdpt_t));
    return 0;
}

void mm_destroy_paging_struct_arch(PagingStruct* ps)
{
    /* This is done in a really stupid way, but it works :) */
    FOR_EACH_ELEM_IN_DARR (ps->allocated_pages, ps->allocated_pages_size, page)
    {
        pte_t* pte = pte_from_vaddr_abs(page->vaddr, ps->data);
        pte_set_frame_paddr(0, pte);
        pte->present = false;

        ffree_one(pte_frame_paddr(pte));
    }

    pt_t* pt = NULL;
    FOR_EACH_ELEM_IN_DARR (ps->allocated_pages, ps->allocated_pages_size, page)
    {
        pt_t* pttmp = pt_from_vaddr_abs(page->vaddr, ps->data);
        if (pt == pttmp)
            continue;

        pt = pttmp;
        kfree(pt);
    }

    pd_t* pd = NULL;
    FOR_EACH_ELEM_IN_DARR (ps->allocated_pages, ps->allocated_pages_size, page)
    {
        pd_t* pdtmp = pd_from_vaddr_abs(page->vaddr, ps->data);
        if (pd == pdtmp)
            continue;

        pd = pdtmp;
        kfree(pd);
    }

    if (ps->data)
    {
        kfree(ps->data);
        ps->data = NULL;
    }
}

int mm_map_kernel_into_paging_struct_arch(PagingStruct* ps)
{
    /* Since the kernel has one page directory pointer table entry just to
     * itself we just copy said entry into this PagingStruct's page directory
     * pointer table. Note that right now, doing this means we also map the 1st
     * MiB of memory into this PagingStruct, which I don't think matters that
     * much as it is cpl 0 memory... */
    pdpt_t* pdpt = PS2PDPT(ps);
    ASSERT(pdpt);

    /* Map the 3-4 GIB region used by the kernel. */
    pdpte_t* kpdpte =
        pdpte_from_vaddr(kimg_vaddr(), PS2PDPT(mm_get_kernel_paging_struct()));
    pdpte_t* pdpte = pdpte_from_vaddr(kimg_vaddr(), pdpt);
    *pdpte = *kpdpte;
    return 0;
}

void mm_load_paging_struct_arch(const PagingStruct* ps)
{
    const ptr new_cr3 = mm_kva2pa((ptr)ps->data);
    if (new_cr3 == cpu_read_cr3())
        return;

    cpu_write_cr3(new_cr3);
}

int mm_map_page_arch(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps)
{
    pdpt_t* pdpt = PS2PDPT(ps);
    ASSERT(pdpt);

    /* Get the page directory pointer table entry. */
    pdpte_t* pdpte = pdpte_from_vaddr(vaddr, pdpt);
    pd_t* pd = pdpte_pd_vaddr(pdpte);
    if (!pd)
    {
        /* There is no pagedir so we allocate it */
        pd = kmalloc_aligned(sizeof(pd_t), PAGESIZE);
        if (!pd)
            return -ENOMEM;

        pd_init(pd);
        pdpte_set_pd_vaddr((ptr)pd, pdpte);
    }

    /* Get the page directory entry */
    pde_t* pde = pde_from_vaddr(vaddr, pd);
    pt_t* pt = pde_pt_vaddr(pde);
    if (!pt)
    {
        /* There is no page table, allocate it */
        pt = kmalloc_aligned(sizeof(pt_t), PAGESIZE);
        if (!pt)
            return -ENOMEM;

        pt_init(pt);
        pde_set_pt_vaddr((ptr)pt, pde);
    }

    /* Get the page table entry. */
    pte_t* pte = pte_from_vaddr(vaddr, pt);
    pte_set_frame_paddr(paddr, pte);

    /* Everything is present */
    pdpte->present = true;
    pde->present = true;
    pte->present = true;

    /* Set flags */
    mm_set_pte_and_pde_flags(pte, pde, flags);
    mm_tlb_flush_single(vaddr);
    return 0;
}

int mm_set_page_flags_arch(ptr vaddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);
    pdpt_t* pdpt = PS2PDPT(ps);
    ASSERT(pdpt);

    pdpte_t* pdpte = pdpte_from_vaddr(vaddr, pdpt);
    pd_t* pd = pdpte_pd_vaddr(pdpte);
    if (!pd)
        return -EINVAL;

    pde_t* pde = pde_from_vaddr(vaddr, pd);
    pt_t* pt = pde_pt_vaddr(pde);
    if (!pt)
        return -EINVAL;

    pte_t* pte = pte_from_vaddr(vaddr, pt);
    ptr frame = pte_frame_paddr(pte);
    if (!frame)
        return -EINVAL;

    mm_set_pte_and_pde_flags(pte, pde, flags);
    mm_tlb_flush_single(vaddr);
    return 0;
}

int mm_rm_page_flags_arch(ptr vaddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);
    pdpt_t* pdpt = PS2PDPT(ps);
    ASSERT(pdpt);

    pte_t* pte = pte_from_vaddr_abs(vaddr, pdpt);
    if (!pte)
        return -ENOENT;

    /* NOTE: We don't demote pde persmissions, should we? */

    pte->exec_disable = !!(flags & PAGE_X);
    pte->writable = !(flags & PAGE_W);
    pte->user_access = !(flags & PAGE_USER);
    pte->present = !(flags & PAGE_PRESENT);
    /* Non-read permissions are not a thing on x86, though they would be nice */
    mm_tlb_flush_single(vaddr);
    return 0;
}