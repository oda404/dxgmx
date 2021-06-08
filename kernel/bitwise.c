
#include<dxgmx/bitwise.h>
#include<dxgmx/gcc/attrs.h>

__ATTR_ALWAYS_INLINE int 
bw_is_aligned(uint64_t n, uint64_t align)
{
    return ((n + (align - 1) & ~(align - 1)) == n);
}

__ATTR_ALWAYS_INLINE void 
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

__ATTR_ALWAYS_INLINE void 
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

__ATTR_ALWAYS_INLINE int
bw_is64_wide(uint64_t n)
{
    return (n >> 32);
}
