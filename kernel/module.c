/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/types.h>

#define KLOGF_PREFIX "module: "

extern u8 _modules_sect_start[];
extern u8 _modules_sect_end[];

static Module* g_modules = NULL;
static size_t g_modules_count = 0;

int module_init_builtins()
{
    g_modules = (Module*)_modules_sect_start;
    g_modules_count =
        ((size_t)_modules_sect_end - (size_t)_modules_sect_start) /
        sizeof(Module);

    FOR_EACH_ELEM_IN_DARR (g_modules, g_modules_count, mod)
    {
        if (!mod->name || !mod->init)
        {
            KLOGF(ERR, "Found fucked-up module at 0x%p", (void*)mod);
            continue;
        }

        KLOGF(INFO, "Found built-in module '%s'.", mod->name);

        if (mod->init)
            mod->init();
    }

    return 0;
}
