
#ifndef _DXGMX_PAGING_KPAGING_H
#define _DXGMX_PAGING_KPAGING_H

#include<dxgmx/mem/pagedir.h>

PageDirectory *kpagedir_get();
int kpaging_init();

#endif //_DXGMX_PAGING_KPAGING_H
