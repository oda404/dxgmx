/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/types.h>

#define KLOGF_PREFIX "module: "

static Module* g_modules = NULL;
static size_t g_modules_count = 0;

int module_init_builtins()
{
    g_modules = (Module*)kimg_module_start();
    g_modules_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    FOR_EACH_ELEM_IN_DARR (g_modules, g_modules_count, mod)
    {
        if (!mod->name || !mod->main)
        {
            KLOGF(ERR, "Found fucked-up module at 0x%p", (void*)mod);
            continue;
        }

        KLOGF(INFO, "Found built-in module '%s'.", mod->name);

        int res = mod->main();
        if (res)
            KLOGF(ERR, "'%s' main() returned with %d.", mod->name, res);
    }

    return 0;
}
