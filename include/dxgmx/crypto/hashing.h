/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_CRYPTO_HASHING_H
#define _DXGMX_CRYPTO_HASHING_H

#include <dxgmx/types.h>

typedef struct HashingFunction
{
    const char* name;

    size_t digest_size;
    int (*chew)(const void* buf, size_t buflen, void* digest);

    u32 security_priority;
    u32 speed_priority;

    size_t refcount;
} HashingFunction;

int hashing_register_hashfunc(const HashingFunction* hashfunc);
int hashing_unregister_hashfunc(const char* name);

HashingFunction* hashing_best_func_by_speed();

#endif // !_DXGMX_CRYPTO_HASHING_H
