/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/kimg.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/module.h>
#include <dxgmx/string.h>
#include <dxgmx/types.h>

#define KLOGF_PREFIX "module: "

static Module* mod_builtin_find_by_name(const char* name)
{
    Module* modules = (Module*)kimg_module_start();
    size_t module_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    FOR_EACH_ELEM_IN_DARR (modules, module_count, mod)
    {
        if (strcmp(mod->name, name) == 0)
            return mod;
    }

    return NULL;
}

static int mod_builtin_init(Module* mod)
{
    if (!mod->name)
    {
        KLOGF(ERR, "Found fucked-up module at 0x%p", (void*)mod);
        return -EINVAL;
    }

    /* Already initialized */
    if (!mod->main)
    {
        KLOGF(ERR, "Module '%s' has no main!", mod->name);
        return 0;
    }

    if (mod->_initialized)
        return 0;

    int st = mod->main();
    if (st < 0)
    {
        KLOGF(ERR, "'%s' returned with %d.", mod->name, st);
        return st;
    }

    mod->_initialized = true;
    return 0;
}

static int mod_builtin_init_stage(ModuleStage stage)
{
    Module* modules = (Module*)kimg_module_start();
    size_t module_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    FOR_EACH_ELEM_IN_DARR (modules, module_count, mod)
    {
        if (mod->stage != stage)
            continue;

        mod_builtin_init(mod);
    }

    return 0;
}

int mod_builtin_init_stage1()
{
    return mod_builtin_init_stage(MODULE_STAGE1);
}

int mod_builtin_init_stage2()
{
    return mod_builtin_init_stage(MODULE_STAGE2);
}

int mod_builtin_init_stage3()
{
    return mod_builtin_init_stage(MODULE_STAGE3);
}

void mod_builtin_dump_all()
{
    Module* modules = (Module*)kimg_module_start();
    size_t module_count =
        (kimg_module_end() - kimg_module_start()) / sizeof(Module);

    const u8 lookup_table[] = {
        [MODULE_STAGE1] = 1,
        [MODULE_STAGE2] = 2,
        [MODULE_STAGE3] = 3 //
    };

    KLOGF(DEBUG, "List of builtin modules:");
    FOR_EACH_ELEM_IN_DARR (modules, module_count, mod)
        KLOGF(DEBUG, "  (%d) %s", lookup_table[mod->stage], mod->name);
}

int mod_builtin_depends_on(const char* mod)
{
    Module* target = mod_builtin_find_by_name(mod);
    if (!target)
        return -ENOENT;

    int st = mod_builtin_init(target);
    if (st < 0)
        return st;

    ++target->_refcount;
    return 0;
}
