/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/proc/proc.h>
#include <dxgmx/proc/procm.h>
#include <dxgmx/x86/gdt.h>

int procm_arch_load_ctx(Process* proc)
{
    tss_set_esp0(proc->kstack_top);
    return 0;
}
