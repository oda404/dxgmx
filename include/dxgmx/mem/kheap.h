
#ifndef _DXGMX_MEM_KHEAP_H
#define _DXGMX_MEM_KHEAP_H

#include<dxgmx/types.h>

bool kheap_init();
size_t kheap_get_size();
ptr kheap_get_start_vaddr();

#endif // !_DXGMX_MEM_KHEAP_H
