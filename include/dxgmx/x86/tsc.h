
#ifndef _DXGMX_X86_TSC_H
#define _DXGMX_X86_TSC_H

#include<dxgmx/types.h>

/** Makes sure that the TSC is enabled. Also calculates the TSC
 * frequency if it is constant or invariant.
*/
int tsc_init();
int tsc_is_available();
int tsc_is_constant();
int tsc_is_invariant();
u64 tsc_read();

#endif //!_DXGMX_X86_TSC_H
