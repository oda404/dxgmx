
#ifndef _DXGMX_MEM_MMANAGER_H
#define _DXGMX_MEM_MMANAGER_H

#include<dxgmx/types.h>
#include<dxgmx/mem/mmap.h>

int mmanager_init();
int mmanager_reserve_acpi_range(ptr base, size_t size);
MemoryMap mmanager_get_sys_mmap();

#endif //!_DXGMX_MEM_MMANAGER_H
