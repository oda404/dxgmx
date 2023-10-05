/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STACK_TRACE_H
#define _DXGMX_STACK_TRACE_H

#include <dxgmx/klog.h>
#include <dxgmx/proc/proc.h>
#include <dxgmx/sched/sched.h>
#include <dxgmx/types.h>

typedef struct S_StackFrame
{
    ptr baseptr;
    ptr instptr;
} StackFrame;

/* Dumps a backtrace of all the function calls. */
void stack_info_dump_trace();
void stack_info_dump_trace_lvl(KLogLevel lvl);

#define STACK_INFO_DUMP_LIMITS() stack_info_dump_limits(__FUNCTION__, __LINE__)

void stack_info_dump_limits(const char* func, int line);

#endif //!_DXGMX_STACK_TRACE_H
