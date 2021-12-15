/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

#if defined(_X86_)
    typedef u32 ptr;
#   define PTR_DIG 8
#elif defined(_X86_64_)
    typedef u64 ptr;
#   define PTR_DIG 16
#endif //_X86_

#define KIB 1024
#define MIB (KIB * 1024)
#define GIB (MIB * 1024)
#define TIB (GIB * 1024)

#endif //!_DXGMX_TYPES_H
