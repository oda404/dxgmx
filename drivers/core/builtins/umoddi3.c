/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* This file was adapted from llvm-project's compiler-rt builtins. */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>

/* count leading zeros - in binary */
#define clz(x) __builtin_clzll(x)
/* duh */
#define U64_BITS 64

_ATTR_NEVER_INLINE u64 __umoddi3(u64 n, u64 d)
{
    // d == 0 cases are unspecified.
    u8 shr = clz(d) - clz(n);
    // 0 <= shr <= (U64_BITS - 1) or shr is very large.
    if (shr > (U64_BITS - 1)) // n < d
        return n;

    if (shr == (U64_BITS - 1)) // d == 1
        return 0;

    ++shr;
    // 1 <= shr <= (U64_BITS - 1). Shifts do not trigger UB.
    u64 r = n >> shr;
    n <<= U64_BITS - shr;
    u64 carry = 0;

    while (shr--)
    {
        r = (r << 1) | (n >> ((U64_BITS - 1)));
        n = (n << 1) | carry;
        // Branch-less version of:
        // carry = 0;
        // if (r >= d) r -= d, carry = 1;
        const i64 s = (i64)(d - r - 1) >> ((U64_BITS - 1));
        carry = s & 1;
        r -= d & s;
    }

    return r;
}
