/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>

#if __SIZEOF_FLOAT__ == __SIZEOF_INT__
typedef int floatwhole;
#elif __SIZEOF_FLOAT__ == __SIZEOF_LONG__
typedef long floatwhole;
#else
typedef long long floatwhole;
#endif // __SIZEOF_FLOAT__ == __SIZEOF_INT__

#if __SIZEOF_DOUBLE__ == __SIZEOF_INT__
typedef int doublewhole;
#elif __SIZEOF_DOUBLE__ == __SIZEOF_LONG__
typedef long doublewhole;
#else
typedef long long doublewhole;
#endif // __SIZEOF_DOUBLE__ == __SIZEOF_INT__

#if __SIZEOF_LONG_DOUBLE__ == __SIZEOF_INT__
typedef int longdoublewhole;
#elif __SIZEOF_LONG_DOUBLE__ == __SIZEOF_LONG__
typedef long longdoublewhole;
#else
typedef long long longdoublewhole;
#endif //__SIZEOF_LONG_DOUBLE__ == __SIZEOF_INT__

#if defined(_X86_)
    typedef uint32_t ptr;
#   define PTR_DIG 8
#elif defined(_X86_64_)
    typedef uint64_t ptr;
#   define PTR_DIG 16
#endif //_X86_

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#define KIB 1024
#define MIB (KIB * 1024)
#define GIB (MIB * 1024)
#define TIB (GIB * 1024)

#endif //!_DXGMX_TYPES_H
