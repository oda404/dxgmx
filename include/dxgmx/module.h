/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MODULE_H
#define _DXGMX_MODULE_H

#include <dxgmx/compiler_attrs.h>

#define MODULE _ATTR_SECTION(".modules") _ATTR_USED

typedef struct S_Module
{
    const char* const name;
    int (*init)();
    void (*destroy)();
} Module;

/* Init all built-in modules */
int module_init_builtins();

#endif // !_DXGMX_MODULE_H
