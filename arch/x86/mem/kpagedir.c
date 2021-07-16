
#include<dxgmx/mem/kpagedir.h>
#include<dxgmx/mem/pagedir.h>
#include<dxgmx/mem/pagetable.h>
#include<dxgmx/mem/paging.h>
#include<dxgmx/mem/pagesize.h>
#include<dxgmx/kprintf.h>
#include<dxgmx/attrs.h>
#include<stddef.h>
#include<stdbool.h>

static PageDirectory pagedir _ATTR_ALIGNED(PAGE_SIZE);
static PageTable pagetables _ATTR_ALIGNED(PAGE_SIZE);

PageDirectory *kpagedir_get()
{
    return &pagedir;
}

int kpagedir_init()
{
    pagedir_init(&pagedir);
    pagetable_init(&pagetables);
    
    /* Identity map the first 4MiB */
    for(size_t i = 0; i < 1024; ++i)
    {
        PageTableEntry *entry = &pagetables.entries[i];
        pagetable_entry_set_frame_base(i * PAGE_SIZE, entry);
        pagetable_entry_set_present(true, entry);
        pagetable_entry_set_rw(true, entry);
    }

    pagedir_entry_set_table_base(&pagetables, &pagedir.entries[0]);
    pagedir_entry_set_present(true, &pagedir.entries[0]);
    pagedir_entry_set_rw(true, &pagedir.entries[0]);

    pagedir_load(&pagedir);
    paging_enable();

    kprintf("Entering paging prison...\n");

    return 0;
}
