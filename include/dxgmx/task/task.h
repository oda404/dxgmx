/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TASK_TASK_H
#define _DXGMX_TASK_TASK_H

#include <dxgmx/generated/kconfig.h>
#include <dxgmx/types.h>

#ifdef CONFIG_X86
#include <dxgmx/x86/task_ctx.h>
#else
#error No kstack context defined
#endif

int task_set_impending_stack_top(ptr sp);
void task_switch(TaskContext* prevct, TaskContext* nextctx);

#endif // !_DXGMX_TASK_TASK_H
