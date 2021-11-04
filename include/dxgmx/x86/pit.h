
#ifndef _DXGMX_X86_PIT_H
#define _DXGMX_X86_PIT_H

#include<dxgmx/time.h>
#include<dxgmx/types.h>

int pit_init();
void pit_enable_periodic_int();
void pit_disable_periodic_int();
bool pit_periodic_ints_enabled();
struct timespec pit_now();

#endif //!_DXGMX_X86_PIT_H
