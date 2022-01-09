/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/mmanager.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/x86/idt.h>
#include<dxgmx/x86/pagedir_ptrtable.h>
#include<dxgmx/x86/pagedir.h>
#include<dxgmx/x86/pagetable.h>
#include<dxgmx/types.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/falloc.h>
#include<dxgmx/mem/memrange.h>
#include<dxgmx/attrs.h>
#include<dxgmx/assert.h>
#include<dxgmx/cpu.h>
#include<dxgmx/string.h>
#include<dxgmx/todo.h>
#include<dxgmx/kmalloc.h>
#include<dxgmx/math.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "mmanager: " fmt, ##__VA_ARGS__)

#define FOR_EACH_PTE_IN_RANGE(s,e,pte) \
for(PageTableEntry *pte = pte_from_vaddr(s); pte; pte = NULL) \
for(ptr _i = s; _i < e; _i += PAGE_SIZE, pte = pte_from_vaddr(_i))

/* The minimum paging structures we need to statically allocate,
before we can use the heap. */
static _ATTR_ALIGNED(PAGE_SIZE) PageTable g_pgtable0;
static _ATTR_ALIGNED(PAGE_SIZE) PageTable g_pgtable1;
static _ATTR_ALIGNED(PAGE_SIZE) PageDirectoryPointerTable g_pdpt;
static _ATTR_ALIGNED(PAGE_SIZE) PageDirectory g_pgdirs[4];

static MemoryMap g_sys_mmap;
static bool g_sys_mmap_locked = false;

static ptr g_heap_start = 0;
static ptr g_heap_size = 0;

extern u8 _kernel_base[];
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

static PageTableEntry *pte_from_vaddr(ptr vaddr)
{
    size_t pte = (size_t)(vaddr / PAGE_SIZE);
    size_t pgtable = pte / 512;
    pte -= pgtable * 512;

    return &g_pgtables[pgtable]->entries[pte];
}

static PageTable *pgtable_from_vaddr(ptr vaddr)
{
    return g_pgtables[(size_t)(vaddr / PAGE_SIZE / 512)];
}

static void paging_isr(
    const InterruptFrame *frame, 
    const void _ATTR_MAYBE_UNUSED *data
)
{
#define PAGEFAULT_IS_PROT_VIOL(x) (x & 1)
#define PAGEFAULT_IS_WRITE(x) (x & (1 << 1))
#define PAGEFAULT_IS_EXEC(x) (x & (1 << 4))

    ptr faultaddr = cpu_read_cr2();

    if(faultaddr < PAGE_SIZE)
        panic("Possible NULL dereference in ring 0 :(. Not proceeding.");

    if(PAGEFAULT_IS_PROT_VIOL(frame->code))
    {
        char msg[10];
        if(PAGEFAULT_IS_EXEC(frame->code))
            strcpy(msg, "exec from");
        else if(PAGEFAULT_IS_WRITE(frame->code))
            strcpy(msg, "write to");
        else
            strcpy(msg, "read from");

        panic("Page protection violation: tried to %s 0x%p. Not proceeding.", msg, (void*)faultaddr);
    }
    else
        TODO_FATAL();
}

_INIT static int setup_definitive_paging()
{
    /* Paging is and has been enabled ever since we jumped to C code
    but we replace the 'boot' paging structs, and set up a heap. */

    memset(&g_pdpt, 0, sizeof(g_pdpt));
    memset(&g_pgdirs, 0, sizeof(g_pgdirs));
    memset(&g_pgtable0, 0, sizeof(g_pgtable0));
    memset(&g_pgtable1, 0, sizeof(g_pgtable1));

    /* Setup the pagefault handler. */
    idt_register_isr(TRAP14, paging_isr);
    /* Enable NXE bit. */
    cpu_write_msr(cpu_read_msr(MSR_EFER) | EFER_NXE, MSR_EFER);

    pdpte_set_pagedir_base((ptr)&g_pgdirs[0] - (ptr)_kernel_map_offset, &g_pdpt.entries[0]);
    pdpte_set_pagedir_base((ptr)&g_pgdirs[1] - (ptr)_kernel_map_offset, &g_pdpt.entries[1]);
    pdpte_set_pagedir_base((ptr)&g_pgdirs[2] - (ptr)_kernel_map_offset, &g_pdpt.entries[2]);
    pdpte_set_pagedir_base((ptr)&g_pgdirs[3] - (ptr)_kernel_map_offset, &g_pdpt.entries[3]);
    g_pdpt.entries[3].present = true;

    PageTable *pgtable = &g_pgtable0;
    if(!pgtable)
        panic("Could not map kernel image. Not proceding.");

    pde_set_table_base((ptr)pgtable - (ptr)_kernel_map_offset, &g_pgdirs[3].entries[0]);
    g_pgdirs[3].entries[0].present = true;
    g_pgdirs[3].entries[0].writable = true;

    /* Map the first 2MiB of memory to 0xC0000000. */
    size_t entries = 256 + ((ptr)_kernel_size / PAGE_SIZE + 1);
    if(entries > 512)
        panic("Kernel has grown beyond 1MiB, how did we get here ?");

    for(size_t i = 0; i < entries; ++i)
    {
        PageTableEntry *entry = &pgtable->entries[i];

        pte_set_frame_base(PAGE_SIZE * i, entry);
        entry->present = true;
        entry->writable = true;
    }

    cpu_write_cr3((ptr)&g_pdpt - (ptr)_kernel_map_offset);

    return 0;
}

_INIT static int setup_heap()
{
    g_heap_start = 0xC0200000;
    g_heap_size = 2 * MIB;

    PageTable *pgtable = &g_pgtable1;
    if(!pgtable)
        panic("Could not map initial heap. Not proceeding.");

    /* Map the heap */
    pde_set_table_base((ptr)pgtable - (ptr)_kernel_map_offset, &g_pgdirs[3].entries[1]);
    g_pgdirs[3].entries[1].present = true;
    g_pgdirs[3].entries[1].writable = true;

    for(size_t i = 1; i <= 512; ++i)
    {
        PageTableEntry *entry = &pgtable->entries[i - 1];

        ptr fbase = falloc(1);
        if(!fbase)
        {
            const size_t initheap = g_heap_size;
            g_heap_size = (i - 1) * 4096;
            KLOGF(WARN, "Only mapped %d/%d pages of the heap.", g_heap_size, initheap);
            break;
        }

        pte_set_frame_base(fbase, entry);
        entry->present = true;
        entry->writable = true;
    }

    cpu_write_cr3((ptr)&g_pdpt - (ptr)_kernel_map_offset);

    return 0;
}

/* Enforce all sections permissions. */
_INIT void enforce_ksections_perms()
{
    /* Text section can't be written to. */
    FOR_EACH_PTE_IN_RANGE((ptr)_text_sect_start, (ptr)_text_sect_end, pte)
        pte->writable = false;

    /* Rodata section can't be written to or executed from. */
    FOR_EACH_PTE_IN_RANGE((ptr)_rodata_sect_start, (ptr)_rodata_sect_end, pte)
    {
        pte->writable = false;
        pte->exec_disable = true;
    }

    /* Can't execute from data. */
    FOR_EACH_PTE_IN_RANGE((ptr)_data_sect_start, (ptr)_data_sect_end, pte)
        pte->exec_disable = true;

    /* Can't execute from bss. */
    FOR_EACH_PTE_IN_RANGE((ptr)_bss_sect_start, (ptr)_bss_sect_end, pte)
        pte->exec_disable = true;

    /* Unmap the bootloader section. */
    FOR_EACH_PTE_IN_RANGE((ptr)_bootloader_sect_start, (ptr)_bootloader_sect_end, pte)
        pte->present = false;

    /* For now only disable execution. */
    FOR_EACH_PTE_IN_RANGE((ptr)_ro_post_init_sect_start, (ptr)_ro_post_init_sect_end, pte)
        pte->exec_disable = true;
}

_INIT int mmanager_init()
{
    setup_definitive_paging();
    enforce_ksections_perms();

    /* First we setup the system memory map to know what
    we're working with. */
    mmap_init(&g_sys_mmap);
    MultibootMBI *mbi = (MultibootMBI *)(_multiboot_info_struct_base + (ptr)_kernel_map_offset);

    for(
        MultibootMMAP *mmap = (MultibootMMAP *)(mbi->mmap_base + (ptr)_kernel_map_offset);
        (ptr)mmap < mbi->mmap_base + (ptr)_kernel_map_offset + mbi->mmap_length;
        mmap = (MultibootMMAP *)((ptr)mmap + mmap->size + sizeof(mmap->size))
    )
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
        &g_sys_mmap
    );

    mmap_align_entries(MMAP_AVAILABLE, PAGE_SIZE, &g_sys_mmap);

    /* Now that we have a memory map, we can start allocating page frames. */
    falloc_init();

    setup_heap();

    /* The heap can finally be initialized. */
    kmalloc_init(g_heap_start, g_heap_size);

    /* ACPI could potentially modify the sys mmap 
    before we lock it down. */
    //acpi_reserve_tables();
    g_sys_mmap_locked = true;

    return 0;
}

_INIT int mmanager_reserve_acpi_range(ptr base, size_t size)
{
    if(g_sys_mmap_locked)
        return 1;

    mmap_update_entry_type(base, size, MMAP_RESERVED, &g_sys_mmap);
    return 0;
}

MemoryMap mmanager_get_sys_mmap()
{
    return g_sys_mmap;
}
