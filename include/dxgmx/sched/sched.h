/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_SCHED_SCHED_H
#define _DXGMX_SCHED_SCHED_H

#include <dxgmx/proc/proc.h>

/** When the kernel is running after spawning pid 1, it's doing so in one of two
 * 'contexts':
 *
 * - On behalf of a process through a syscall, or some sort of fault.
 *
 * - Servicing a hardware IRQ.
 *
 * If we are servicing a hardware IRQ, this function is not really helpful. But
 * if we are running on behalf of a process, there are a lot of places where we
 * need to know what that process is. This is what this function gets us. The
 * last process that ran, before we got here.
 *
 * NOTE: There is no way for the kernel to know if it's actually coming from a
 * user process after it left it's 'entry point'. For now we are just assuming
 * the kernel is coming from a process in a few places.
 * Also this function doesn't even begin to consider the ideea of SMP.
 * Processes is running on a single CPU and that is it
 */
Process* sched_current_proc();

/* Initialize the scheduler. This function starts running kinit_stage3 and in
 * turn initializes the userspace. This function does not return. */
_ATTR_NORETURN void sched_init();

/* Switch to next process waiting to run. This function does not return. */
_ATTR_NORETURN void sched_yield();

#endif // !_DXGMX_SCHED_SCHED_H
