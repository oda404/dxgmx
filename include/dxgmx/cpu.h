/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_CPU_H
#define _DXGMX_CPU_H

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/generated/kconfig.h>
#include <dxgmx/types.h>

#if defined(CONFIG_X86)
#include <dxgmx/bits/x86/cpu.h>
#else
#error "Unknown CPU arch"
#endif // defined(CONFIG_X86_)

/** Tries to identify the CPU and it's features. */
int cpu_identify();
/** Suspends CPU execution. Note that the CPU might resume
 * execution due to various architecture specific reasons.
 */
void cpu_suspend();
/**
 * Terminates cpu execution. This function will never return.
 */
_ATTR_NORETURN void cpu_hang();
/**
 * Returns a const* to a CPUInfo struct with architecture specific info.
 * If cpu_identify was not called prior to this function, it will return NULL.
 */
const CPUInfo* cpu_get_info();

bool cpu_has_feature(CPUFeatureFlag flag);

#endif // _DXGMX_CPU_H
