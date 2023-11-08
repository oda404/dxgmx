/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_PAGEFAULT_H
#define _DXGMX_MEM_PAGEFAULT_H

#include <dxgmx/types.h>

typedef enum PageFaultReason
{
    PAGEFAULT_REASON_PROT_FAULT,
    PAGEFAULT_REASON_ABSENT
} PageFaultReason;

typedef enum PageFaultAction
{
    PAGEFAULT_ACTION_READ,
    PAGEFAULT_ACTION_WRITE,
    PAGEFAULT_ACTION_EXEC
} PageFaultAction;

ptr pagefault_handle(
    ptr faultaddr,
    ptr ip,
    u8 ring,
    PageFaultReason reason,
    PageFaultAction action);

#endif // !_DXGMX_MEM_PAGEFAULT_H
