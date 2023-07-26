/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/dma.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/heap.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/multiboot.h>
#include <dxgmx/x86/pagefault.h>
#include <dxgmx/x86/pd.h>
#include <dxgmx/x86/pdpt.h>
#include <dxgmx/x86/pt.h>

#define KLOGF_PREFIX "mm: "

/* mm exposes this for cases where something platform agnostic needs the kernel
 * paging struct. g_kernel_paging_struct.data == g_pdpt */
static PagingStruct g_kernel_paging_struct;
static pdpt_t* g_pdpt;

#define FOR_EACH_PTE_IN_RANGE(s, e, pt, pte)                                   \
    for (pte_t* pte = pte_from_vaddr(s, pt); pte; pte = NULL)                  \
        for (ptr _i = s; _i < e; _i += PAGESIZE, pte = pte_from_vaddr(_i, pt))

#define FOR_EACH_KPTE_IN_RANGE(s, e, pte)                                      \
    for (pte_t* pte = pte_from_vaddr_abs(s, g_pdpt); pte; pte = NULL)          \
        for (ptr _i = s; _i < e;                                               \
             _i += PAGESIZE, pte = pte_from_vaddr_abs(_i, g_pdpt))

#define SYS_MEMORY_REGIONS_MAX 16
static MemoryRegion g_sys_mregs[SYS_MEMORY_REGIONS_MAX];
static MemoryRegionMap g_sys_mregmap = {
    .regions = g_sys_mregs,
    .regions_size = 0,
    .regions_capacity = SYS_MEMORY_REGIONS_MAX};

static void mm_tlb_flush_whole()
{
    cpu_write_cr3(cpu_read_cr3());
}

static void mm_tlb_flush_single(ptr vaddr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

/* Returns the vaddr of any paddr that is part of the kernel. Since the kernel
 * image is mapped 1:1 to wherever it's loaded, this is as simple
 * as adding the kernel's map offset to the paging struct paddr. */

static pd_t* pd_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    ptr pd_base = pdpte_pd_paddr(&pdpt->entries[vaddr / GIB]);
    return pd_base ? (void*)mm_kpaddr2vaddr(pd_base) : NULL;
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
    return pt_base ? (void*)mm_kpaddr2vaddr(pt_base) : NULL;
}

static pte_t* pte_from_vaddr_abs(ptr vaddr, const pdpt_t* pdpt)
{
    pt_t* pt = pt_from_vaddr_abs(vaddr, pdpt);
    return pt ? pte_from_vaddr(vaddr, pt) : NULL;
}

static ptr mm_vaddr2paddr_arch(ptr vaddr, const pdpt_t* pdpt)
{
    pte_t* pte = pte_from_vaddr_abs(vaddr, pdpt);
    return pte ? (ptr)pte_frame_paddr(pte) + vaddr % PAGESIZE : (ptr)NULL;
}

static void mm_set_page_flags_arch(pte_t* pte, pde_t* pde, u16 flags)
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

/**
 * Map a PAGESIZEd page.
 *
 * 'vaddr' PAGESIZE aligned virtual address of the page to be mapped.
 * 'padd' PAGESIZE aligned physical address of the page frame to be used for
 * this mapping.
 * 'flags' Access and control flags.
 * 'ps' Target paging structure.
 *
 * Returns:
 * 0 on sucess
 * -EINVAL on invalid arguments.
 * -ENOMEM on out of memory.
 */
static int mm_map_page_arch(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps)
{
    pdpt_t* pdpt = ps->data;
    if (!pdpt)
        return -EINVAL;

    /* Get the page directory pointer table entry. */
    pdpte_t* pdpte = pdpte_from_vaddr(vaddr, pdpt);

    /* Get the page directory */
    pd_t* pd = pdpte_pd_vaddr(pdpte);
    if (!pd)
    {
        /* There is no pagedir so we allocate it */
        pd = kmalloc_aligned(sizeof(PageDirectory), PAGESIZE);
        if (!pd)
            return -ENOMEM;

        pd_init(pd);
        pdpte_set_pd_vaddr((ptr)pd, pdpte);
    }

    /* Get the page directory entry */
    pde_t* pde = pde_from_vaddr(vaddr, pd);

    /* Get the page table */
    pt_t* pt = pde_pt_vaddr(pde);
    if (!pt)
    {
        /* There is no page table, allocate it */
        pt = kmalloc_aligned(sizeof(PageTable), PAGESIZE);
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
    mm_set_page_flags_arch(pte, pde, flags);

    mm_tlb_flush_single(vaddr);

    return 0;
}

static _INIT int mm_setup_definitive_paging()
{
    /* Enable NXE bit. */
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);

    g_pdpt = (void*)mm_kpaddr2vaddr(cpu_read_cr3());
    g_kernel_paging_struct.data = g_pdpt;

    return 0;
}

/* Enforce all sections permissions. */
_INIT static void mm_enforce_ksections_perms()
{
    /* Unmap the bootloader section. */
    FOR_EACH_KPTE_IN_RANGE (kimg_bootloader_start(), kimg_bootloader_end(), pte)
        pte->present = false;

    /* Text section can't be written to. */
    FOR_EACH_KPTE_IN_RANGE (kimg_text_start(), kimg_text_end(), pte)
        pte->writable = false;

    /* Can't write to modules */
    FOR_EACH_KPTE_IN_RANGE (kimg_module_start(), kimg_module_end(), pte)
    {
        /* We leave modules as writable, because they get altered after
         * initialization. */
        pte->exec_disable = true;
    }

    /* Can't write to kinit_stage3 */
    FOR_EACH_KPTE_IN_RANGE (
        kimg_kinit_stage3_text_start(), kimg_kinit_stage3_text_end(), pte)
    {
        pte->writable = false;
    }

    /* Can't write to init, which is just text */
    FOR_EACH_KPTE_IN_RANGE (kimg_init_start(), kimg_init_end(), pte)
        pte->writable = false;

    /* Can't execute from data. */
    FOR_EACH_KPTE_IN_RANGE (kimg_data_start(), kimg_data_end(), pte)
        pte->exec_disable = true;

    /* For now only disable execution. */
    FOR_EACH_KPTE_IN_RANGE (
        kimg_ro_postinit_start(), kimg_ro_postinit_end(), pte)
        pte->exec_disable = true;

    /* Rodata section can't be written to or executed from. */
    FOR_EACH_KPTE_IN_RANGE (kimg_rodata_start(), kimg_rodata_end(), pte)
    {
        pte->writable = false;
        pte->exec_disable = true;
    }

    /* Can't execute from bss. */
    FOR_EACH_KPTE_IN_RANGE (kimg_bss_start(), kimg_bss_end(), pte)
        pte->exec_disable = true;

    /* Can't write or execute from ksyms */
    FOR_EACH_KPTE_IN_RANGE (kimg_ksyms_start(), kimg_ksyms_end(), pte)
    {
        pte->writable = false;
        pte->exec_disable = true;
    }

    mm_tlb_flush_whole();
}

static _INIT bool mm_setup_sys_mregmap()
{
    /* Zero out areas. */
    memset(
        g_sys_mregmap.regions,
        0,
        g_sys_mregmap.regions_capacity * sizeof(MemoryRegion));

    MultibootMBI* mbi =
        (MultibootMBI*)(_multiboot_info_struct_base + kimg_map_offset());

    size_t total_regions = 0;
    for (MultibootMMAP* mmap =
             (MultibootMMAP*)(mbi->mmap_base + kimg_map_offset());
         (ptr)mmap < mbi->mmap_base + kimg_map_offset() + mbi->mmap_length;
         mmap = (MultibootMMAP*)((ptr)mmap + mmap->size + sizeof(mmap->size)))
    {
        if (total_regions > SYS_MEMORY_REGIONS_MAX)
            panic("Hit maximum number of system memory regions!");

        if (mmap->type != MULTIBOOT_MMAP_TYPE_AVAILABLE)
            continue;

        const MemoryRegion newreg = {
            .start = mmap->base, .size = mmap->length, .perms = MEM_REGION_RWX};
        mregmap_add_reg(&newreg, &g_sys_mregmap);
        ++total_regions;
    }

    klogln(INFO, "Available memory provided by firmware:");
    FOR_EACH_MEM_REGION (region, &g_sys_mregmap)
    {
        klogln(
            INFO,
            "  [mem 0x%p-0x%p].",
            (void*)region->start,
            (void*)(region->start + region->size - 1));
    }

    /* Remove the 1st MiB */
    mregmap_rm_reg(0, MIB, &g_sys_mregmap);
    klogln(INFO, "Reserving [mem 0x%p-0x%p].", (void*)0, (void*)(MIB - 1));
    /* Remove the kernel image. */
    mregmap_rm_reg(kimg_paddr(), kimg_size(), &g_sys_mregmap);
    klogln(
        INFO,
        "Reserving [mem 0x%p-0x%p].",
        (void*)kimg_paddr(),
        (void*)(kimg_paddr() + kimg_size() - 1));

    mregmap_align_regs(PAGESIZE, &g_sys_mregmap);

    return true;
}

/* The kernel heap is the memory starting right after the kernel image, both in
 * physical & virtual space, and stopping at ???. The reason for doing this
 * instead of using the whole address space, is that translations to physical
 * addresses are trivial (vaddr - _kernel_map_offset) and allocating phsyical &
 * virtual contiguous pages is also easy. Basically everything that is part of
 * the kernel is contiguous in physical & virtual memory. There is a nice
 * diagram of the system address space when it comes to page frames in falloc.c
 * :)
 */
static _INIT int mm_setup_kmalloc()
{
    /* Since the PageTable that holds the kernel image (and the 1st MIB of
     memory, total of 2 MIB) is completely mapped to physical memory
     ([0 - 2MiB] -> [_kernel_map_offset - (_knernel_map_offset + 2 MIB)]),
     depending on KHEAP_SIZE, parts of the kheap may not be mapped to physical
     memory. In case the kheap does fall outside the kernel pagetable, we do try
     to handle pagefaults for it, but they are kind of hack-ish and may panic
     the kernel
     :(. See arch/x86/mem/pagefault.c for more info. */

    const size_t kheap_init_size = (1 * MIB);
    ptr kernel_vend = kimg_vaddr() + kimg_size();
    ASSERT(kernel_vend % PAGESIZE == 0);

    Heap kheap = {.vaddr = kernel_vend, .pagespan = kheap_init_size / PAGESIZE};

    char unit[4];
    u32 heapsize = bytes_to_human_readable(kheap.pagespan * PAGESIZE, unit);
    KLOGF(
        DEBUG,
        "kheap start: 0x%p, size: %d %s.",
        (void*)kheap.vaddr,
        heapsize,
        unit);

    int st = kmalloc_register_kernel_heap(kheap);
    if (st < 0)
        return st;

    /* Make sure the kernel heap is zeroed out. We always start mapping
    memory from 0 to where ever the kernel end is, in 2 MiB increments (whole
    Page tables). Let's say the kernel starts at 1 MiB and is around ~324 KiB
    the rest of ~700 KiB are considered kernel heap, and are still mapped. Note
    that new kernel heap pages allocated when a pagefault happens are
    automatically zeroed out. */
    size_t sz = bytes_align_up64(kheap.vaddr, 0x200'000) - kheap.vaddr;
    memset((void*)kheap.vaddr, 0, sz);

    /* Mark these pages in falloc */
    for (size_t i = 0; i < sz / PAGESIZE; ++i)
    {
        ptr paddr = mm_kvaddr2paddr(kheap.vaddr + i * PAGESIZE);
        falloc_one_at(paddr);
    }

    /* We won't panic the kernel in case kmalloc can't be initialized, just for
     * debugging. But otherwise the kernel is pretty useless
     * without being able to allocate memory. */
    if (kmalloc_init() < 0)
        KLOGF(WARN, "Failed to initialize kmalloc!");

    return 0;
}

_INIT int mm_init()
{
    /* Sets up any stuff that wasn't done in entry.S */
    mm_setup_definitive_paging();

    /* Start getting pagefaults. */
    pagefault_setup_isr();

    /* Enforce permissions for all kernel sections. */
    mm_enforce_ksections_perms();

    /* Setup the system memory map */
    mm_setup_sys_mregmap();

    /* Initialize the frame allocator, depends on the system memory map. */
    falloc_init();

    /* Setup kmalloc */
    mm_setup_kmalloc();

    /* Initialize DMA subsystem */
    dma_init();

    return 0;
}

int mm_init_paging_struct(PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    int st = pagingstruct_init(ps);
    if (st < 0)
        return st;

    ps->data = kmalloc_aligned(sizeof(PageDirectoryPointerTable), PAGESIZE);
    if (!ps->data)
    {
        pagingstruct_destroy(ps);
        return -ENOMEM;
    }

    memset(ps->data, 0, sizeof(PageDirectoryPointerTable));

    return 0;
}

int mm_destroy_paging_struct(PagingStruct* ps)
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

    kfree(ps->data);
    ps->data = NULL;

    pagingstruct_destroy(ps);

    return 0;
}

int mm_set_page_flags(ptr vaddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);

    pdpt_t* pdpt = ps->data;
    if (!pdpt)
        return -EINVAL;

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

    mm_set_page_flags_arch(pte, pde, flags);

    mm_tlb_flush_single(vaddr);

    return 0;
}

int mm_new_page(ptr vaddr, ptr paddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);
    ASSERT(paddr % PAGESIZE == 0);

    return mm_map_page_arch(vaddr, paddr, flags, ps);
}

int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct* ps)
{
    ASSERT(vaddr % PAGESIZE == 0);

    ptr paddr = falloc_one_user();
    if (!paddr)
        return -ENOMEM;

    int st = mm_new_page(vaddr, paddr, flags | PAGE_USER, ps);
    if (st < 0)
        ffree_one(paddr);

    Page page = {.vaddr = vaddr};
    st = pagingstruct_track_page(&page, ps);
    if (st < 0)
        ASSERT_NOT_HIT(); // FIXME: can't be fucked.

    return st;
}

int mm_map_kernel_into_paging_struct(PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    /* Since the kernel has one page directory pointer table entry just to
     * itself we just copy said entry into this PagingStruct's page directory
     * pointer table. Note that right now, doing this means we also map the 1st
     * MiB of memory into this PagingStruct, which I don't think matters that
     * much as it is cpl 0 memory... */
    pdpt_t* pdpt = (pdpt_t*)ps->data;
    if (!pdpt)
        return -EINVAL;

    /* Map the 3-4 GIB region used by the kernel. */
    pdpte_t* kpdpte = pdpte_from_vaddr(kimg_vaddr(), g_pdpt);
    pdpte_t* pdpte = pdpte_from_vaddr(kimg_vaddr(), pdpt);
    *pdpte = *kpdpte;

    mm_tlb_flush_single(kimg_vaddr());

    /* Map the 0-1 GIB region used by DMA. */
    kpdpte = pdpte_from_vaddr(DMA_POOL_START, g_pdpt);
    pdpte = pdpte_from_vaddr(DMA_POOL_START, pdpt);
    *pdpte = *kpdpte;

    mm_tlb_flush_single(DMA_POOL_START);

    return 0;
}

int mm_load_paging_struct(const PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    /* The kernel better be mapped into this :) */
    cpu_write_cr3(mm_vaddr2paddr_arch((ptr)ps->data, g_pdpt));
    return 0;
}

int mm_load_kernel_paging_struct()
{
    cpu_write_cr3(mm_vaddr2paddr_arch((ptr)g_pdpt, g_pdpt));
    return 0;
}

const MemoryRegionMap* mm_get_sys_mregmap()
{
    return &g_sys_mregmap;
}

PagingStruct* mm_get_kernel_paging_struct()
{
    return &g_kernel_paging_struct;
}

ptr mm_kpaddr2vaddr(ptr paddr)
{
    return paddr + kimg_map_offset();
}

ptr mm_vaddr2paddr(ptr vaddr, const PagingStruct* ps)
{
    return mm_vaddr2paddr_arch(vaddr, ps->data);
}

ptr mm_kvaddr2paddr(ptr vaddr)
{
    return vaddr - kimg_map_offset();
}
