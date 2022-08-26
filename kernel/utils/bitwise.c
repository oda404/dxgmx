/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/compiler_attrs.h>
#include <dxgmx/utils/bitwise.h>

_ATTR_ALWAYS_INLINE void bw_clear(u64* n, u8 bit)
{
    if (bit < 32)
    {
        u32 x = ~(1 << bit);
        (*n) &= x;
    }
    else if (bit < 64)
    {
        u32 old = (*n);
        (*n) >>= 32;
        u32 x = ~(1 << (bit - 32));
        (*n) &= x;
        (*n) <<= 32;
        (*n) |= old;
    }
}

_ATTR_ALWAYS_INLINE void bw_set(u64* n, u8 bit)
{
    if (bit < 32)
    {
        u32 x = (1 << bit);
        (*n) |= x;
    }
    else if (bit < 64)
    {
        u32 old = (*n);
        (*n) >>= 32;
        u32 x = (1 << (bit - 32));
        (*n) |= x;
        (*n) <<= 32;
        (*n) |= old;
    }
}

_ATTR_ALWAYS_INLINE u64 bw_mask(u64 n, u64 mask)
{
    u64 ret;
    ret = (n >> 32) & (mask >> 32);
    ret <<= 32;
    u32 tmp = (u32)n & (u32)mask;
    ret |= tmp;

    return ret;
}

_ATTR_ALWAYS_INLINE void bw_or_mask(u64* n, u64 mask)
{
    u32 old = (u32)(*n);
    (*n) >>= 32;
    u32 tmp = (mask >> 32);
    (*n) |= tmp;
    (*n) <<= 32;
    tmp = old | (u32)mask;
    (*n) |= tmp;
}

_ATTR_ALWAYS_INLINE void bw_and_mask(u64* n, u64 mask)
{
    u32 old = (u32)(*n);
    (*n) >>= 32;
    u32 tmp = (mask >> 32);
    (*n) &= tmp;
    (*n) <<= 32;
    tmp = old & (u32)mask;
    (*n) |= tmp;
}

_ATTR_ALWAYS_INLINE int bw_is64_wide(u64 n)
{
    return (n >> 32);
}

_ATTR_ALWAYS_INLINE u32 bw_u32_rotl(u32 n, u8 rot)
{
    return ((n << rot) | (n >> (32 - rot)));
}

_ATTR_ALWAYS_INLINE u32 bw_u32_flip_endianness(u32 n)
{
    return ((n >> 24) & 0XFF) | ((n << 8) & 0xFF0000) | ((n >> 8) & 0xFF00) |
           ((n << 24) & 0xFF000000);
}

_ATTR_ALWAYS_INLINE bool bw_is_power_of_two(u64 n)
{
    return n != 0 && (n & (n - 1)) == 0;
}
