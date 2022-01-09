/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/utils/bitwise.h>
#include<dxgmx/compiler_attrs.h>

_ATTR_ALWAYS_INLINE void 
bw_clear(uint64_t *n, uint8_t bit)
{
    if(bit < 64)
    {
        u32 old = *n;
        (*n) >>= bit;
        (*n) &= ~1;
        (*n) <<= bit;
        (*n) |= old;
    }
}

_ATTR_ALWAYS_INLINE void 
bw_set(uint64_t *n, uint8_t bit)
{
    if(bit < 64)
    {
        u32 old = *n;
        (*n) >>= bit;
        (*n) |= 1;
        (*n) <<= bit;
        (*n) |= old;
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

_ATTR_ALWAYS_INLINE bool 
bw_is_power_of_two(u64 n)
{
    u8 setbits = 0;
    for(size_t i = 0; i < 64; ++i)
        setbits += (n >> i) & 1;

    return setbits == 1;
}
