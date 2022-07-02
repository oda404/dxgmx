/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* This file was adapted from llvm-project's compiler-rt builtins. */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

/* duh */
#define U64_BITS 64

extern u64 __udivdi3(u64 n, u64 d);

_ATTR_NEVER_INLINE i64 __divdi3(i64 a, i64 b)
{
    i64 s_a = a >> (U64_BITS - 1);               // s_a = a < 0 ? -1 : 0
    i64 s_b = b >> (U64_BITS - 1);               // s_b = b < 0 ? -1 : 0
    u64 a_u = (u64)(a ^ s_a) + (-s_a);           // negate if s_a == -1
    u64 b_u = (u64)(b ^ s_b) + (-s_b);           // negate if s_b == -1
    s_a ^= s_b;                                  // sign of quotient
    return (__udivdi3(a_u, b_u) ^ s_a) + (-s_a); // negate if s_a == -1
}
