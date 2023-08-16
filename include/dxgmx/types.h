/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include <dxgmx/err_or.h>
#include <posix/sys/types.h>

#define NULL ((void*)0)

typedef __UINT8_TYPE__ u8;
typedef __UINT16_TYPE__ u16;
typedef __UINT32_TYPE__ u32;
typedef __UINT64_TYPE__ u64;
typedef __INT8_TYPE__ i8;
typedef __INT16_TYPE__ i16;
typedef __INT32_TYPE__ i32;
typedef __INT64_TYPE__ i64;

typedef __UINTPTR_TYPE__ ptr;

#define KIB 1024
#define MIB (KIB * 1024)
#define GIB (MIB * 1024)
#define TIB (GIB * 1024)

#if (__STDC_VERSION__ <= 201710L) || (!defined(__clang__)) ||                  \
    (defined(__clang__) && __clang_major__ < 15)
#include <stdbool.h>
#endif

/* For each loop to simplify looping over a dynamic array. */
#define FOR_EACH_ELEM_IN_DARR(arr, count, elem)                                \
    for (__typeof__(arr) elem = arr; elem < arr + count; ++elem)

DEFINE_ERR_OR(ptr);
DEFINE_ERR_OR_PTR(char);

#endif //!_DXGMX_TYPES_H
