
#include<dxgmx/mem/mmanager.h>
#include<dxgmx/mem/mmap.h>
#include<dxgmx/x86/multiboot.h>
#include<dxgmx/types.h>
#include<dxgmx/klog.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/mem/pgframe_alloc.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/kinfo.h>
#include<dxgmx/mem/memrange.h>

#define KLOGF(lvl, fmt, ...) klog(lvl, "mmanager: " fmt, ##__VA_ARGS__)

static MemoryMap g_mmap;

int mmanager_init()
{
    mmap_init(&g_mmap);

    MultibootMBI *mbi = (MultibootMBI *)_multiboot_info_struct_base;

    for(
        MultibootMMAP *mmap = (MultibootMMAP *)mbi->mmap_base;
        (ptr)mmap < mbi->mmap_base + mbi->mmap_length;
        mmap = (MultibootMMAP *)((ptr)mmap + mmap->size + sizeof(mmap->size))
    )
    {
        mmap_add_entry(mmap->base, mmap->length, mmap->type);
    }

    KLOGF(KLOG_INFO, "Memory map provided by BIOS:\n");
    mmap_dump();

    mmap_update_entry_type(0, PAGE_SIZE, MMAP_RESERVED);
    /* mark the kernel itself as kreserved */
    mmap_update_entry_type(
        kinfo_get_kbase(), 
        kinfo_get_kend() - kinfo_get_kbase(), 
        MMAP_RESERVED
    );

    mmap_align_entries(PAGE_SIZE);

    pgframe_alloc_init();
    paging_init();

    return 0;
}
