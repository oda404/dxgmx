
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/types.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/falloc.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/mem/memrange.h>
#include<dxgmx/attrs.h>
#include<dxgmx/assert.h>

#define KLOGF(lvl, fmt, ...) klogln(lvl, "mmanager: " fmt, ##__VA_ARGS__)

static MemoryMap g_sys_mmap;
static bool g_sys_mmap_locked = false;

extern u8 _kernel_base[];
extern u8 _kernel_end[];
extern u8 _text_section_base[];
extern u8 _text_section_end[];
extern u8 _rodata_section_base[];
extern u8 _rodata_section_end[];
extern u8 _data_section_base[];
extern u8 _data_section_end[];
extern u8 _bss_section_base[];
extern u8 _bss_section_end[];

_INIT static void mmanager_enforce_kernel_section_perms()
{
    ASSERT((ptr)_text_section_base % PAGE_SIZE == 0);
    ASSERT((ptr)_rodata_section_base % PAGE_SIZE == 0);
    ASSERT((ptr)_data_section_base % PAGE_SIZE == 0);
    ASSERT((ptr)_bss_section_base % PAGE_SIZE == 0);

    PageTableEntry *pte = NULL;

    for(ptr i = (ptr)_text_section_base; i < (ptr)_text_section_end; i += PAGE_SIZE)
    {
        pte = paging_pte_from_vaddr(i);
        pte->writable = false;
        paging_flush_tlb_entries(i);
    }

    for(ptr i = (ptr)_rodata_section_base; i < (ptr)_rodata_section_end; i += PAGE_SIZE)
    {
        pte = paging_pte_from_vaddr(i);
        pte->writable = false;
        pte->exec_disable = true;
        paging_flush_tlb_entries(i);
    }

    for(ptr i = (ptr)_data_section_base; i < (ptr)_data_section_end; i += PAGE_SIZE)
    {
        pte = paging_pte_from_vaddr(i);
        pte->exec_disable = true;
        paging_flush_tlb_entries(i);
    }

    for(ptr i = (ptr)_bss_section_base; i < (ptr)_bss_section_end; i += PAGE_SIZE)
    {
        pte = paging_pte_from_vaddr(i);
        pte->exec_disable = true;
        paging_flush_tlb_entries(i);
    }
}

_INIT int mmanager_init()
{
    mmap_init(&g_sys_mmap);
    MultibootMBI *mbi = (MultibootMBI *)_multiboot_info_struct_base;

    for(
        MultibootMMAP *mmap = (MultibootMMAP *)mbi->mmap_base;
        (ptr)mmap < mbi->mmap_base + mbi->mmap_length;
        mmap = (MultibootMMAP *)((ptr)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_add_entry(mmap->base, mmap->length, mmap->type, &g_sys_mmap);
    }

    KLOGF(INFO, "Memory map provided by BIOS:");
    mmap_dump(&g_sys_mmap);

    mmap_update_entry_type(0, PAGE_SIZE, MMAP_RESERVED, &g_sys_mmap);
    /* mark the kernel itself as kreserved */
    mmap_update_entry_type(
        (ptr)_kernel_base, 
        (size_t)_kernel_end - (size_t)_kernel_base, 
        MULTIBOOT_MMAP_TYPE_RESERVED,
        &g_sys_mmap
    );

    mmap_align_entries(MMAP_AVAILABLE, PAGE_SIZE, &g_sys_mmap);

    /* ACPI could potentially modify the sys mmap 
    before we lock it down. */
    acpi_reserve_tables();
    g_sys_mmap_locked = true;

    falloc_init();
    paging_init();

    mmanager_enforce_kernel_section_perms();

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
