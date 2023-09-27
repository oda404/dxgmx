/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/proc/proc.h>
#include <dxgmx/x86/gdt.h>

int proc_arch_load_ctx(const Process* proc)
{
    tss_set_esp0(proc->kstack_top);
    return 0;
}

ptr proc_arch_get_current_kstack_top()
{
    return tss_get_esp0();
}
