
#ifndef _DXGMX_KINFO_H
#define _DXGMX_KINFO_H

#include<dxgmx/kstack.h>
#include<dxgmx/types.h>

ptr kinfo_get_kbase();
ptr kinfo_get_kend();
ptr kinfo_get_kstack_top();
ptr kinfo_get_kstack_bot();

#endif //!_DXGMX_KINFO_H
