/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MODULE_H
#define _DXGMX_MODULE_H

#include <dxgmx/compiler_attrs.h>

/**
 * There are 3 levels of builin modules in the dxgmx kernel:
 *
 * stage1:
 *  The memory manager has not been initialized, you can not allocate any
 * memory. There is no CPU information, no kernel output, and no timers. The
 * only thing that ran before this point was some early architecture specific
 * setup code. This is mostly for kernel output sinks.
 *
 * stage2:
 *  There is now a memory manager, CPU information, kernel symbols and klog.
 * There are however no timers. Now would be a good stage for system timers and
 * system buses.
 *
 * stage3:
 *  Timers, and any configured system buses are now up. Now would be a good time
 * for hardware drivers. The reason for having another level here is because,
 * hardware drivers obviously need system buses, and would probably want to use
 * timers for stuff like timeouts. The problem is that we can't really
 * initialize system timers/buses before these modules, if they were on the same
 * level.
 */

#define MODULE_ATTRS _ATTR_SECTION(".modules") _ATTR_USED
#define MODULE static MODULE_ATTRS const Module

typedef enum E_ModuleStage
{
    MODULE_STAGE3 = 0,
    MODULE_STAGE1,
    MODULE_STAGE2,
} ModuleStage;

typedef struct S_Module
{
    const char* const name;

    /* Load stage for built in module. */
    ModuleStage stage;

    int (*main)();
    int (*exit)();

    bool _initialized;
    size_t _refcount;
} Module;

int mod_builtin_init_stage1();
int mod_builtin_init_stage2();
int mod_builtin_init_stage3();

void mod_builtin_dump_all();

int mod_builtin_depends_on(const char* mod);

#endif // !_DXGMX_MODULE_H
