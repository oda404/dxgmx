
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/x86/acpi.h>
#include<dxgmx/types.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/pgframe_alloc.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/kinfo.h>
#include<dxgmx/mem/memrange.h>
#include<dxgmx/attrs.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "mmanager: " fmt, ##__VA_ARGS__)

static MemoryMap g_sys_mmap;
static bool g_sys_mmap_locked = false;

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

    KLOGF(KLOG_INFO, "Memory map provided by BIOS:\n");
    mmap_dump(&g_sys_mmap);

    mmap_update_entry_type(0, PAGE_SIZE, MMAP_RESERVED, &g_sys_mmap);
    /* mark the kernel itself as kreserved */
    mmap_update_entry_type(
        kinfo_get_kbase(), 
        kinfo_get_kend() - kinfo_get_kbase(), 
        MULTIBOOT_MMAP_TYPE_RESERVED,
        &g_sys_mmap
    );

    mmap_align_entries(MMAP_AVAILABLE, PAGE_SIZE, &g_sys_mmap);

    /* ACPI could potentially modify the sys mmap 
    before we lock it down. */
    acpi_reserve_tables();
    g_sys_mmap_locked = true;

    pgframe_alloc_init();
    paging_init();

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
