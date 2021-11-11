
#ifndef _DXGMX_STACK_TRACE_H
#define _DXGMX_STACK_TRACE_H

#include<dxgmx/types.h>

typedef struct 
S_StackFrame
{
    ptr baseptr;
    ptr instptr;
} StackFrame;

/* Dumps a backtrace of all the function calls. */
void stack_trace_dump();

#endif //!_DXGMX_STACK_TRACE_H
