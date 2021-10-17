/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/bitwise.h>
#include<dxgmx/attrs.h>

_ATTR_ALWAYS_INLINE int 
bw_is_aligned(uint64_t n, uint64_t align)
{
    return (((n + (align - 1)) & ~(align - 1)) == n);
}

_ATTR_ALWAYS_INLINE void 
bw_clear(uint64_t *n, uint8_t bit)
{
    if(bit < 32)
    {
        *((uint32_t *)n) &= ~(1 << bit);
    }
    else if(bit < 64)
    {
        uint32_t half = (uint32_t)(*n);
        *n >>= 32;
        *((uint32_t *)n) &= ~(1 << (bit - 32));
        *n <<= 32;
        *((uint32_t *)n) = half;
    }
}

_ATTR_ALWAYS_INLINE void 
bw_set(uint64_t *n, uint8_t bit)
{
    if(bit < 32)
    {
        *((uint32_t *)n) |= 1 << bit;
    }
    else if(bit < 64)
    {
        uint32_t half = (uint32_t)(*n);
        *n >>= 32;
        *((uint32_t *)n) |= 1 << (bit - 32);
        *n <<= 32;
        *((uint32_t *)n) = half;
    }
}

_ATTR_ALWAYS_INLINE int
bw_is64_wide(uint64_t n)
{
    return (n >> 32);
}

_ATTR_ALWAYS_INLINE u32 
bw_u32_rotl(u32 n, u8 rot)
{
    return ((n << rot) | (n >> (32 - rot)));
}

_ATTR_ALWAYS_INLINE u32
bw_u32_flip_endianness(u32 n)
{
    return ((n >> 24) & 0XFF) | ((n << 8) & 0xFF0000) | ((n >> 8) & 0xFF00) | ((n << 24) & 0xFF000000);
}
