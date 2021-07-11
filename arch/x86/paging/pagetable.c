
#include<dxgmx/paging/pagetable.h>
#include<dxgmx/string.h>

void pagetable_init(PageTable *table)
{
    memset((void *)table, 0, sizeof(table));
}
