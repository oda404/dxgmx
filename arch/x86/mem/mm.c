/**
 * Copyright 2022 Alexandru Olaru.
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
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/heap.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/utils/bytes.h>
#include <dxgmx/x86/acpi.h>
#include <dxgmx/x86/multiboot.h>
#include <dxgmx/x86/pagedir.h>
#include <dxgmx/x86/pagedir_ptrtable.h>
#include <dxgmx/x86/pagefault.h>
#include <dxgmx/x86/pagetable.h>

#define KLOGF_PREFIX "mm: "

static PageDirectoryPointerTable* g_pdpt;

#define FOR_EACH_PTE_IN_RANGE(s, e, pt, pte)                                   \
    for (PageTableEntry* pte = pte_from_vaddr(s, pt); pte; pte = NULL)         \
        for (ptr _i = s; _i < e; _i += PAGESIZE, pte = pte_from_vaddr(_i, pt))

#define FOR_EACH_KPTE_IN_RANGE(s, e, pte)                                      \
    for (PageTableEntry* pte = pte_from_vaddr_abs(s, g_pdpt); pte; pte = NULL) \
        for (ptr _i = s; _i < e;                                               \
             _i += PAGESIZE, pte = pte_from_vaddr_abs(_i, g_pdpt))

#define SYS_MEMORY_REGIONS_MAX 16
static MemoryRegion g_sys_mregs[SYS_MEMORY_REGIONS_MAX];
static MemoryRegionMap g_sys_mregmap = {
    .regions = g_sys_mregs,
    .regions_size = 0,
    .regions_capacity = SYS_MEMORY_REGIONS_MAX};

void mm_tlb_flush_whole()
{
    cpu_write_cr3(cpu_read_cr3());
}

void mm_tlb_flush_single(ptr vaddr)
{
    __asm__ volatile("invlpg (%0)" : : "r"(vaddr) : "memory");
}

/* Returns the vaddr of any paddr that is part of the kernel. Since the kernel
 * image is mapped 1:1 to wherever it's loaded, this is as simple
 * as adding the kernel's map offset to the paging struct paddr. */
static void* mm_kpaddr2vaddr(ptr paddr)
{
    return (void*)(paddr + kimg_map_offset());
}

PageDirectory*
pd_from_vaddr_abs(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    ptr pd_base = pdpte_pagedir_base(&pdpt->entries[vaddr / GIB]);
    return pd_base ? mm_kpaddr2vaddr(pd_base) : NULL;
}

PageDirectoryEntry*
pde_from_vaddr_abs(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    PageDirectory* pd = pd_from_vaddr_abs(vaddr, pdpt);
    return pd ? pde_from_vaddr(vaddr, pd) : NULL;
}

PageTable* pt_from_vaddr_abs(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    pde_t* pde = pde_from_vaddr_abs(vaddr, pdpt);
    if (!pde)
        return NULL;

    ptr pt_base = pde_table_base(pde);
    return pt_base ? mm_kpaddr2vaddr(pt_base) : NULL;
}

PageTableEntry*
pte_from_vaddr_abs(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    PageTable* pt = pt_from_vaddr_abs(vaddr, pdpt);
    return pt ? pte_from_vaddr(vaddr, pt) : NULL;
}

ptr mm_vaddr2paddr(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    PageTableEntry* pte = pte_from_vaddr_abs(vaddr, pdpt);
    return pte ? (ptr)pte_frame_base(pte) + vaddr % PAGESIZE : (ptr)NULL;
}

ptr mm_kvaddr2paddr(ptr vaddr)
{
    return mm_vaddr2paddr(vaddr, g_pdpt);
}

/**
 * Maps a single 4KiB page from virtual vaddr to physical frame_base.
 */
static PageTableEntry*
map_page(ptr frame_base, ptr vaddr, PageDirectoryPointerTable* pdpt)
{
    if (frame_base % PAGESIZE || vaddr % PAGESIZE)
        return NULL;

    PageDirectoryPointerTableEntry* pdpte = pdpte_from_vaddr(vaddr, pdpt);

    PageDirectory* pd = NULL;
    ptr pd_base = pdpte_pagedir_base(pdpte);
    if (!pd_base)
    {
        pd = kmalloc_aligned(sizeof(PageDirectory), PAGESIZE);
        if (!pd)
            return NULL;

        pagedir_init(pd);
        pdpte_set_pagedir_base(mm_vaddr2paddr((ptr)pd, g_pdpt), pdpte);
    }
    else
    {
        pd = mm_kpaddr2vaddr(pd_base);
    }

    PageDirectoryEntry* pde = pde_from_vaddr(vaddr, pd);

    pt_t* pt = NULL;
    ptr pt_base = pde_table_base(pde);
    if (!pt_base)
    {
        pt = kmalloc_aligned(sizeof(PageTable), PAGESIZE);
        if (!pt)
            return NULL;

        pagetable_init(pt);
        pde_set_table_base(mm_vaddr2paddr((ptr)pt, g_pdpt), pde);
    }
    else
    {
        pt = mm_kpaddr2vaddr(pt_base);
    }

    PageTableEntry* pte = pte_from_vaddr(vaddr, pt);
    pte_set_frame_base(frame_base, pte);

    pdpte->present = true;

    pde->present = true;

    pte->present = true;

    mm_tlb_flush_single(vaddr);

    return pte;
}

_INIT static int setup_definitive_paging()
{
    /* Enable NXE bit. */
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);

    g_pdpt = (PageDirectoryPointerTable*)mm_kpaddr2vaddr(cpu_read_cr3());

    return 0;
}

/* Enforce all sections permissions. */
_INIT static void enforce_ksections_perms()
{
    /* Unmap the bootloader section. */
    FOR_EACH_KPTE_IN_RANGE (kimg_bootloader_start(), kimg_bootloader_end(), pte)
        pte->present = false;

    /* Text section can't be written to. */
    FOR_EACH_KPTE_IN_RANGE (kimg_text_start(), kimg_text_end(), pte)
        pte->writable = false;

    /* Can't write to modules */
    FOR_EACH_KPTE_IN_RANGE (kimg_module_start(), kimg_module_end(), pte)
        pte->writable = false;

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

/* The kernel heap is the memory starting right after the kernel image and
 * stopping at ???. The reason for doing this instead of
 * using the whole address space, is that translations to physical addresses are
 * trivial (vaddr - _kernel_map_offset). */
static _INIT int mm_setup_kernel_heap()
{
    /* Since the PageTable that holds the kernel image (and the 1st MIB of
     memory, total of 2 MIB) is completely mapped to physical memory
     ([0 - 2MiB] -> [_kernel_map_offset - (_knernel_map_offset + 2 MIB)]),
     depending on KHEAP_SIZE, parts of the kheap may not be mapped to physical
     memory. In case the kheap does fall outside the kernel pagetable, we do try
     to handle pagefaults for it, but they are kind of hack-ish and may panic
     the kernel
     :(. See arch/x86/mem/pagefault.c for more info. */

#define KHEAP_SIZE (1 * MIB)

    ptr kernel_vend = kimg_vaddr() + kimg_size();
    size_t heap_size = KHEAP_SIZE;

    Heap kheap = {.vaddr = kernel_vend, .pagespan = heap_size / PAGESIZE};
    ASSERT(kernel_vend % PAGESIZE == 0);

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

    /* FIXME: Any kernel heap pages that fall inside the initial kernel
     * pagetable, should really be marked in the memory region map, just to make
     * sure falloc doesn't do anything stupid. */

    return 0;
}

_INIT int mm_init()
{
    pagefault_setup_isr();

    setup_definitive_paging();
    enforce_ksections_perms();

    /* First we setup the system memory regions map to know what
    we're working with. */
    mm_setup_sys_mregmap();

    /* Now that we have a memory map, we can start allocating page frames. */
    falloc_init();

    mm_setup_kernel_heap();

    /* We won't panic the kernel in case kmalloc can't be initialized, just for
     * debugging. But otherwise the kernel is pretty useless
     * without being able to allocate memory. */
    if (kmalloc_init() != 0)
        KLOGF(WARN, "Failed to initialize kmalloc!");

    /* FIXME: Really not the mm's job */
    acpi_reserve_tables();

    return 0;
}

PageDirectoryPointerTable* mm_kernel_pdpt()
{
    return g_pdpt;
}

_INIT int mm_reserve_acpi_range(ptr base, size_t size)
{
    ptr aligned_base = base - base % PAGESIZE;
    size_t aligned_size = size;
    aligned_size = (aligned_size + PAGESIZE - 1) & ~(PAGESIZE - 1);

    /* FIXME: a new page gets allocated everytime, discarding the previous valid
     * one! */
    PageTableEntry* pte = map_page(aligned_base, aligned_base, g_pdpt);
    if (!pte)
        return -ENOMEM;

    mregmap_rm_reg(base, size, &g_sys_mregmap);

    return 0;
}

int mm_init_paging_struct(PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    ps->data = kmalloc_aligned(sizeof(PageDirectoryPointerTable), PAGESIZE);
    if (!ps->data)
        return -ENOMEM;

    return 0;
}

int mm_destroy_paging_struct(PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    /* Here we should free all memory owned by this PagingStruct. Freeing this
     * exact struct is easy, but freeing any other memory used by this struct
     * (any kmalloced Page(Directories/Tables) and any pageframes used) is the
     * *hard* part, just because right now I have no good ideea of how to do so.
     * A bad? ideea tho, would be storing the lower & upper addresses used by
     * this paging struct, and walking the PageDirectory(ies) corresponding to
     * that memory range, freeing everything on the way. Or maybe I'm just
     * looking at this the wrong way... */
    TODO();

    return 0;
}

int mm_new_user_page(ptr vaddr, u16 flags, PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    PageDirectoryPointerTable* pdpt = ps->data;
    if (!pdpt)
        return -EINVAL;

    ptr paddr = falloc_one_user();
    if (!paddr)
        return -ENOMEM;

    PageTableEntry* pte = map_page(paddr, vaddr, pdpt);
    if (!pte)
    {
        ffree_one(paddr);
        return -ENOMEM;
    }

    PageDirectoryEntry* pde = pde_from_vaddr_abs(vaddr, pdpt);
    ASSERT(pde);

    (void)flags;
    pde->user_access = 1;
    pde->writable = 1;

    pte->user_access = 1;
    pte->writable = 1;

    mm_tlb_flush_single(vaddr);

    return 0;
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
    PageDirectoryPointerTable* pdpt = (PageDirectoryPointerTable*)ps->data;
    if (!pdpt)
        return -EINVAL;

    PageDirectoryPointerTableEntry* kpdpte =
        pdpte_from_vaddr(kimg_vaddr(), g_pdpt);

    PageDirectoryPointerTableEntry* pdpte =
        pdpte_from_vaddr(kimg_vaddr(), pdpt);

    *pdpte = *kpdpte;

    return 0;
}

int mm_load_paging_struct(PagingStruct* ps)
{
    if (!ps)
        return -EINVAL;

    /* The kernel better be mapped into this :) */
    cpu_write_cr3(mm_vaddr2paddr((ptr)ps->data, g_pdpt));
    return 0;
}

int mm_load_kernel_paging_struct()
{
    cpu_write_cr3(mm_vaddr2paddr((ptr)g_pdpt, g_pdpt));
    return 0;
}

const MemoryRegionMap* mm_get_sys_mregmap()
{
    return &g_sys_mregmap;
}
