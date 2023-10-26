/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_X86_TASK_CTX_H
#define _DXGMX_X86_TASK_CTX_H

#include <dxgmx/types.h>

typedef struct TaskContext
{
    ptr stack_ptr;
} TaskContext;

#endif // !_DXGMX_X86_TASK_CTX_H
