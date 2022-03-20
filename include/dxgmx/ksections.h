
#ifndef _DXGMX_KSECTIONS_H
#define _DXGMX_KSECTIONS_H

#include <dxgmx/types.h>

ptr ksections_get_text_start();
ptr ksections_get_text_end();
ptr ksections_get_kstack_top();
ptr ksections_get_kstack_bot();
ptr ksections_get_init_start();
ptr ksections_get_init_end();

#endif // !_DXGMX_KSECTIONS_H
