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

static int modbuiltin_init(Module* mod, Module* initialmod);

static Module* modbuiltin_find_by_name(const char* name)
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

static int modbuiltin_resolve_dependency_by_name(
    const char* name, Module* dependant, Module* initialmod)
{
    if (strlen(name) == 0)
    {
        KLOGF(ERR, "Bad dependency list for module '%s'.", dependant->name);
        return -EINVAL;
    }

    if (strcmp(name, initialmod->name) == 0)
    {
        KLOGF(
            ERR,
            "Detected dependency cycle for module '%s'.",
            initialmod->name);
        return -EINVAL;
    }

    Module* dep = modbuiltin_find_by_name(name);
    if (!dep)
    {
        KLOGF(
            ERR,
            "Could not resolve dependency '%s' of module '%s'.",
            dep->name,
            dependant->name);
        return -ENOENT;
    }

    int st = modbuiltin_init(dep, initialmod);
    if (st < 0)
    {
        KLOGF(
            ERR,
            "Dependency '%s' of module '%s' failed with code: %d.",
            dep->name,
            dependant->name,
            st);
        return st;
    }

    return 0;
}

static int modbuiltin_resolve_dependencies(Module* mod, Module* initialmod)
{
    char depname[64] = {0};
    size_t running_len = 0;

    const size_t depsize = strlen(mod->dependencies);
    for (size_t i = 0; i < depsize; ++i)
    {
        if (mod->dependencies[i] == ',')
        {
            int st =
                modbuiltin_resolve_dependency_by_name(depname, mod, initialmod);

            if (st < 0)
                return st;

            running_len = 0;
            memset(depname, 0, 64);

            continue;
        }

        if (running_len >= 63)
        {
            KLOGF(
                ERR,
                "Can't store the name of a dependency of module '%s', skipping.",
                mod->name);
            return -ERANGE;
        }

        strncat(depname, mod->dependencies + i, 1);
        ++running_len;
    }

    /* We have one more. */
    return modbuiltin_resolve_dependency_by_name(depname, mod, initialmod);
}

static int modbuiltin_init(Module* mod, Module* initialmod)
{
    if (!mod->name)
    {
        KLOGF(ERR, "Found fucked-up module at 0x%p", (void*)mod);
        return -EINVAL;
    }

    /* Already initialized */
    if (!mod->main)
        return 0;

    if (mod->dependencies)
    {
        int st = modbuiltin_resolve_dependencies(mod, initialmod);
        if (st < 0)
            return st;
    }

    int st = mod->main();
    if (st < 0)
    {
        KLOGF(ERR, "'%s' returned with %d.", mod->name, st);
        return st;
    }

    mod->main = NULL;
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

        modbuiltin_init(mod, mod);
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
