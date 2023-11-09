/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_TYPES_H
#define _DXGMX_TYPES_H

#include <dxgmx/err_or.h>
#include <dxgmx/posix/sys/types.h>
#include <dxgmx/units.h>
#include <dxgmx/user@types.h>

#define NULL ((void*)0)

typedef _u8 u8;
typedef _u16 u16;
typedef _u32 u32;
typedef _u64 u64;
typedef _i8 i8;
typedef _i16 i16;
typedef _i32 i32;
typedef _i64 i64;

typedef __UINTPTR_TYPE__ ptr;

#if (__STDC_VERSION__ <= 201710L) || (!defined(__clang__)) ||                  \
    (defined(__clang__) && __clang_major__ < 15)
#include <stdbool.h>
#endif

/* For each loop to simplify looping over a dynamic array. */
#define FOR_EACH_ELEM_IN_DARR(arr, count, elem)                                \
    for (__typeof__(arr) elem = arr; elem < arr + count; ++elem)

#if __has_builtin(__builtin_offsetof)
#define OFFSETOF(_struct, _memb) __builtin_offsetof(_struct, _memb)
#else
#define OFFSETOF(_struct, _memb) ((size_t)(&((_struct*)0)->_memb))
#endif

DEFINE_ERR_OR(ptr);
DEFINE_ERR_OR_PTR(char);
DEFINE_ERR_OR(u8);
DEFINE_ERR_OR_PTR(void);
DEFINE_ERR_OR(size_t);

#endif //!_DXGMX_TYPES_H
