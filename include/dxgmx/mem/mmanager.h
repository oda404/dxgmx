
#ifndef _DXGMX_MEM_MMANAGER_H
#define _DXGMX_MEM_MMANAGER_H

#include<dxgmx/types.h>
#include<dxgmx/mem/mmap.h>

int mmanager_init();
MemoryMap mmanager_get_sys_mmap();

#if defined(_X86_)
int mmanager_reserve_acpi_range(ptr base, size_t size);
#endif

#endif //!_DXGMX_MEM_MMANAGER_H
