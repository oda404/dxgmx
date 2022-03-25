/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_MMANAGER_H
#define _DXGMX_MEM_MMANAGER_H

#include <dxgmx/mem/mregmap.h>
#include <dxgmx/types.h>

int mmanager_init();
/* Get the system memory region map. This map only includes
regions that are available. */
const MemoryRegionMap* mmanager_get_sys_mregmap();

#if defined(_X86_)
int mmanager_reserve_acpi_range(ptr base, size_t size);
#endif

#endif //!_DXGMX_MEM_MMANAGER_H
