/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/mem/mm.h>
#include <dxgmx/module.h>
#include <dxgmx/string.h>
#include <dxgmx/types.h>

#define KLOGF_PREFIX "module: "

static int module_init(Module* mod);

static int module_init(Module* mod)
{
    if (!mod->name)
    {
        KLOGF(ERR, "Found fucked-up module at 0x%p", (void*)mod);
        return -EINVAL;
    }

    int st = mod->main();
    if (st < 0)
    {
        KLOGF(ERR, "'%s' returned with %d.", mod->name, st);
        return st;
    }

    return 0;
}

static int modules_init_stage(ModuleStage stage)
{
    Module* modules = (Module*)kimg_module_start();
    size_t module_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    FOR_EACH_ELEM_IN_DARR (modules, module_count, mod)
    {
        if (mod->stage != stage)
            continue;

        module_init(mod);
    }

    return 0;
}

int modules_init_stage1()
{
    return modules_init_stage(MODULE_STAGE1);
}

int modules_init_stage2()
{
    return modules_init_stage(MODULE_STAGE2);
}

int modules_init_stage3()
{
    return modules_init_stage(MODULE_STAGE3);
}

void modules_dump_builtins()
{
    Module* modules = (Module*)kimg_module_start();
    size_t module_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    KLOGF(DEBUG, "List of builtin modules:");
    FOR_EACH_ELEM_IN_DARR (modules, module_count, mod)
        KLOGF(DEBUG, "-- \"%s\", stage %d.", mod->name, mod->stage);
}
