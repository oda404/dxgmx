
#ifndef _DXGMX_KMALLOC_H
#define _DXGMX_KMALLOC_H

#include<dxgmx/types.h>

int kmalloc_init();
void *kmalloc(size_t size);
void kfree(void *addr);
void *krealloc(void *addr, size_t size);

#endif //!_DXGMX_KMALLOC_H
