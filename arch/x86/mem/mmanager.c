/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/attrs.h>
#include <dxgmx/cpu.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/mem/falloc.h>
#include <dxgmx/mem/kheap.h>
#include <dxgmx/mem/memrange.h>
#include <dxgmx/mem/mmanager.h>
#include <dxgmx/mem/mmap.h>
#include <dxgmx/mem/pagesize.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/acpi.h>
#include <dxgmx/x86/idt.h>
#include <dxgmx/x86/multiboot.h>
#include <dxgmx/x86/pagedir.h>
#include <dxgmx/x86/pagedir_ptrtable.h>
#include <dxgmx/x86/pagetable.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "mmanager: " fmt, ##__VA_ARGS__)

#define RET_IF_NOT(x, ret)                                                     \
    if (!(x))                                                                  \
        return ret;

#define FOR_EACH_PTE_IN_RANGE(s, e, pt, pte)                                   \
    for (PageTableEntry* pte = pte_from_vaddr(s, pt); pte; pte = NULL)         \
        for (ptr _i = s; _i < e; _i += PAGE_SIZE, pte = pte_from_vaddr(_i, pt))

static PageDirectoryPointerTable* g_pdpt;

static MemoryMap g_sys_mmap;
static bool g_sys_mmap_locked = false;

extern u8 _kernel_base[];
extern u8 _kernel_vaddr[];
extern u8 _text_sect_start[];
extern u8 _text_sect_end[];
extern u8 _rodata_sect_start[];
extern u8 _rodata_sect_end[];
extern u8 _ro_post_init_sect_start[];
extern u8 _ro_post_init_sect_end[];
extern u8 _init_sect_start[];
extern u8 _init_sect_end[];
extern u8 _data_sect_start[];
extern u8 _data_sect_end[];
extern u8 _bss_sect_start[];
extern u8 _bss_sect_end[];
extern u8 _bootloader_sect_start[];
extern u8 _bootloader_sect_end[];
extern u8 _kernel_map_offset[];
extern u8 _kernel_size[];

#if defined(_X86_)

static _ATTR_NEVER_INLINE void tlb_flush_whole()
{
    cpu_write_cr3(cpu_read_cr3());
}

static _ATTR_ALWAYS_INLINE PageDirectoryPointerTableEntry*
pdpte_from_vaddr(ptr vaddr, PageDirectoryPointerTable* pdpt)
{
    return &pdpt->entries[vaddr / GIB];
}

static _ATTR_ALWAYS_INLINE PageDirectoryEntry*
pde_from_vaddr(ptr vaddr, PageDirectory* pd)
{
    ptr off = vaddr / (PAGE_SIZE * 512 * 512) * 512;
    return &pd->entries[(vaddr / (PAGE_SIZE * 512)) - off];
}

static _ATTR_ALWAYS_INLINE PageTableEntry*
pte_from_vaddr(ptr vaddr, PageTable* pt)
{
    ptr off = vaddr / (PAGE_SIZE * 512) * 512;
    return &pt->entries[(vaddr / PAGE_SIZE) - off];
}

static ptr vaddr_to_physaddr(ptr vaddr, const PageDirectoryPointerTable* pdpt)
{
    PageDirectory* pd =
        (PageDirectory*)((ptr)pdpte_pagedir_base(&pdpt->entries[vaddr / GIB]) + (ptr)_kernel_map_offset);
    RET_IF_NOT(pd, 0);

    PageDirectoryEntry* pde = pde_from_vaddr(vaddr, pd);
    RET_IF_NOT(pde, 0);

    PageTable* pt = pde_table_base(pde);
    RET_IF_NOT(pt, 0);

    PageTableEntry* pte =
        (PageTableEntry*)((ptr)pte_from_vaddr(vaddr, pt) + (ptr)_kernel_map_offset);

    return pte ? (ptr)pte_frame_base(pte) + vaddr % PAGE_SIZE : (ptr)NULL;
}

/**
 * Maps a single 4KiB page from virtual vaddr to physical frame_base.
 */
static PageTableEntry*
map_page(ptr frame_base, ptr vaddr, PageDirectoryPointerTable* pdpt)
{
    if (frame_base % PAGE_SIZE || vaddr % PAGE_SIZE)
        return NULL;

    PageDirectoryPointerTableEntry* pdpte = pdpte_from_vaddr(vaddr, pdpt);
    PageDirectory* pd = pdpte_pagedir_base(pdpte);
    if (!pd)
    {
        pd = kmalloc_aligned(sizeof(PageDirectory), PAGE_SIZE);
        RET_IF_NOT(pd, NULL);

        memset(pd, 0, sizeof(PageDirectory));
        pdpte_set_pagedir_base(vaddr_to_physaddr((ptr)pd, pdpt), pdpte);
    }
    else
    {
        pd = (PageDirectory*)((ptr)pd + (ptr)_kernel_map_offset);
    }

    PageDirectoryEntry* pde = pde_from_vaddr(vaddr, pd);
    PageTable* pt = pde_table_base(pde);
    if (!pt)
    {
        pt = kmalloc_aligned(sizeof(PageTable), PAGE_SIZE);
        RET_IF_NOT(pt, NULL);

        memset(pt, 0, sizeof(PageTable));
        pde_set_table_base(vaddr_to_physaddr((ptr)pt, pdpt), pde);
    }
    else
    {
        pt = (PageTable*)((ptr)pt + (ptr)_kernel_map_offset);
    }

    PageTableEntry* pte = pte_from_vaddr(vaddr, pt);
    pte_set_frame_base(frame_base, pte);

    pdpte->present = true;

    pde->present = true;

    pte->present = true;

    tlb_flush_whole();

    return pte;
}
#endif // defined(_X86_)

static void
pagefault_isr(const InterruptFrame* frame, const void _ATTR_MAYBE_UNUSED* data)
{
#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

    ptr faultaddr = cpu_read_cr2();

    if (faultaddr < PAGE_SIZE)
        panic("Possible NULL dereference in ring 0 :(. Not proceeding.");

    if (PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        const char* msg = NULL;
        if (PAGEFAULT_IS_EXEC(frame->code))
            msg = "exec from";
        else if (PAGEFAULT_IS_WRITE(frame->code))
            msg = "write to";
        else
            msg = "read from";

        panic(
            "Page protection violation: tried to %s 0x%p. Not proceeding.",
            msg,
            (void*)faultaddr);
    }
    else
        TODO_FATAL();
}

_INIT static int setup_definitive_paging()
{
    /* Setup the pagefault handler. */
    idt_register_isr(TRAP14, pagefault_isr);
    /* Enable NXE bit. */
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);

    g_pdpt =
        (PageDirectoryPointerTable*)(cpu_read_cr3() + (ptr)_kernel_map_offset);

    return 0;
}

/* Enforce all sections permissions. */
_INIT static void enforce_ksections_perms()
{
    PageDirectoryPointerTableEntry* pdpte =
        pdpte_from_vaddr((ptr)_kernel_vaddr, g_pdpt);
    PageDirectory* pd =
        (PageDirectory*)((ptr)pdpte_pagedir_base(pdpte) + (ptr)_kernel_map_offset);
    PageDirectoryEntry* pde = pde_from_vaddr((ptr)_kernel_vaddr, pd);
    PageTable* pt =
        (PageTable*)((ptr)pde_table_base(pde) + (ptr)_kernel_map_offset);

    /* Text section can't be written to. */
    FOR_EACH_PTE_IN_RANGE((ptr)_text_sect_start, (ptr)_text_sect_end, pt, pte)
    pte->writable = false;

    /* Rodata section can't be written to or executed from. */
    FOR_EACH_PTE_IN_RANGE(
        (ptr)_rodata_sect_start, (ptr)_rodata_sect_end, pt, pte)
    {
        pte->writable = false;
        pte->exec_disable = true;
    }

    /* Can't execute from data. */
    FOR_EACH_PTE_IN_RANGE((ptr)_data_sect_start, (ptr)_data_sect_end, pt, pte)
    pte->exec_disable = true;

    /* Can't execute from bss. */
    FOR_EACH_PTE_IN_RANGE((ptr)_bss_sect_start, (ptr)_bss_sect_end, pt, pte)
    pte->exec_disable = true;

    /* Unmap the bootloader section. */
    FOR_EACH_PTE_IN_RANGE(
        (ptr)_bootloader_sect_start, (ptr)_bootloader_sect_end, pt, pte)
    pte->present = false;

    /* For now only disable execution. */
    FOR_EACH_PTE_IN_RANGE(
        (ptr)_ro_post_init_sect_start, (ptr)_ro_post_init_sect_end, pt, pte)
    pte->exec_disable = true;
}

_INIT int mmanager_init()
{
    setup_definitive_paging();
    enforce_ksections_perms();

    /* First we setup the system memory map to know what
    we're working with. */
    mmap_init(&g_sys_mmap);
    MultibootMBI* mbi =
        (MultibootMBI*)(_multiboot_info_struct_base + (ptr)_kernel_map_offset);

    for (MultibootMMAP* mmap =
             (MultibootMMAP*)(mbi->mmap_base + (ptr)_kernel_map_offset);
         (ptr)mmap <
         mbi->mmap_base + (ptr)_kernel_map_offset + mbi->mmap_length;
         mmap = (MultibootMMAP*)((ptr)mmap + mmap->size + sizeof(mmap->size)))
    {
        mmap_add_entry(mmap->base, mmap->length, mmap->type, &g_sys_mmap);
    }

    KLOGF(INFO, "Memory map provided by BIOS:");
    mmap_dump(&g_sys_mmap);

    /* Reserve the 1st MiB */
    mmap_update_entry_type(0, MIB, MMAP_RESERVED, &g_sys_mmap);
    /* Reserve the kernel image. */
    mmap_update_entry_type(
        (ptr)_kernel_base,
        (ptr)_kernel_size,
        MULTIBOOT_MMAP_TYPE_RESERVED,
        &g_sys_mmap);

    mmap_align_entries(MMAP_AVAILABLE, PAGE_SIZE, &g_sys_mmap);

    /* Now that we have a memory map, we can start allocating page frames. */
    falloc_init();

    kheap_init();

    /* The heap can finally be initialized. */
    kmalloc_init();

    /* ACPI could potentially modify the sys mmap
    before we lock it down. */
    acpi_reserve_tables();
    g_sys_mmap_locked = true;

    return 0;
}

_INIT int mmanager_reserve_acpi_range(ptr base, size_t size)
{
    if (g_sys_mmap_locked)
        return 1;

    ptr aligned_base = base - base % PAGE_SIZE;
    size_t aligned_size = size;
    aligned_size = (aligned_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    map_page(aligned_base, aligned_base, g_pdpt);

    mmap_update_entry_type(base, size, MMAP_RESERVED, &g_sys_mmap);

    return 0;
}

MemoryMap mmanager_get_sys_mmap()
{
    return g_sys_mmap;
}
