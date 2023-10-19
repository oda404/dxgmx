/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/crypto/hashing.h>
#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/string.h>

static HashingFunction* g_hash_funcs;
static size_t g_hash_func_count;

static HashingFunction* hashing_new_hashfunc()
{
    HashingFunction* tmp = krealloc(
        g_hash_funcs, (g_hash_func_count + 1) * sizeof(HashingFunction));

    if (!tmp)
        return NULL;

    g_hash_funcs = tmp;
    ++g_hash_func_count;
    return &g_hash_funcs[g_hash_func_count - 1];
}

static int hashing_shrink_hashfuncs()
{
    HashingFunction* tmp = krealloc(
        g_hash_funcs, (g_hash_func_count - 1) * sizeof(HashingFunction));

    if (!tmp)
        return -ENOMEM; // ?

    g_hash_funcs = tmp;
    --g_hash_func_count;
    return 0;
}

int hashing_register_hashfunc(const HashingFunction* hashfunc)
{
    HashingFunction* newfunc = hashing_new_hashfunc();
    if (!newfunc)
        return -ENOMEM;

    *newfunc = *hashfunc;
    return 0;
}

int hashing_unregister_hashfunc(const char* name)
{
    HashingFunction* func = NULL;
    for (size_t i = 0; i < g_hash_func_count; ++i)
    {
        if (strcmp(g_hash_funcs[i].name, name) == 0)
        {
            func = &g_hash_funcs[i];
            break;
        }
    }

    if (func->refcount)
        return -EBUSY;

    for (HashingFunction* f = func; f < g_hash_funcs + g_hash_func_count - 1;
         ++f)
        *f = *(f + 1);

    hashing_shrink_hashfuncs();
    return 0;
}

HashingFunction* hashing_best_func_by_speed()
{
    if (!g_hash_funcs)
        return NULL;

    HashingFunction* func = &g_hash_funcs[0];
    for (size_t i = 1; i < g_hash_func_count; ++i)
    {
        if (g_hash_funcs[i].speed_priority > func->speed_priority)
            func = &g_hash_funcs[i];
    }

    return func;
}
