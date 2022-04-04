/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#if defined(_X86_)
typedef i32 ssize_t;
#elif defined(_X86_64_)
typedef i64 ssize_t;
#endif

#if defined(_X86_)
typedef u32 ptr;
#define PTR_DIG 8
#elif defined(_X86_64_)
typedef u64 ptr;
#define PTR_DIG 16
#endif //_X86_

#define KIB 1024
#define MIB (KIB * 1024)
#define GIB (MIB * 1024)
#define TIB (GIB * 1024)

#define typeof __typeof__ // we out here (re)defining keywords :)

/* For each loop to simplify looping over a dynamic array. */
#define FOR_EACH_ELEM_IN_DARR(arr, count, elem)                                \
    for (typeof(arr) elem = arr; elem < arr + count; ++elem)

#endif //!_DXGMX_TYPES_H
