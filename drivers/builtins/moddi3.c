/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* This file was adapted from llvm-project's compiler-rt builtins. */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

/* duh */
#define U64_BITS 64

extern u64 __umoddi3(u64 n, u64 d);

_ATTR_NEVER_INLINE i64 __moddi3(i64 a, i64 b)
{
    i64 s = b >> (U64_BITS - 1);   // s = b < 0 ? -1 : 0
    u64 b_u = (u64)(b ^ s) + (-s); // negate if s == -1
    s = a >> (U64_BITS - 1);       // s = a < 0 ? -1 : 0
    u64 a_u = (u64)(a ^ s) + (-s); // negate if s == -1
    u64 res = __umoddi3(a_u, b_u);
    return (res ^ s) + (-s); // negate if s == -1
}
