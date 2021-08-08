/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/mem/paging.h>
#include<dxgmx/cpu.h>

void paging_enable()
{
    cpu_set_cr0(cpu_get_cr0() | CR0FLAG_PE | CR0FLAG_PG | CR0FLAG_WP);
}
