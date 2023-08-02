/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_POSIX_SYS_TYPES_H
#define _DXGMX_POSIX_SYS_TYPES_H

#include <stdint.h>

typedef __SIZE_TYPE__ size_t;

/* ssize_t */
#if __SIZE_WIDTH__ == 32
typedef int32_t ssize_t;

#ifdef _KERNEL
#define _SSIZE_MAX_ 0x7fffffff
#define PLATFORM_MAX_UNSIGNED 0xFFFFFFFF
#endif // _KERNEL

#elif __SIZE_WIDTH__ == 64
typedef int64_t ssize_t;

#ifdef _KERNEL
#define _SSIZE_MAX_ 0x7fffffffffffffff
#define PLATFORM_MAX_UNSIGNED 0xFFFFFFFFFFFFFFFF
#endif // _KERNEL

#else
#error "Weird __SIZE_WIDTH__!"
#endif

typedef size_t mode_t;

typedef uint32_t dev_t;
typedef size_t ino_t;

typedef uint32_t nlink_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef uint32_t id_t;

typedef ssize_t off_t;

typedef ssize_t blksize_t;
typedef ssize_t blkcnt_t;

typedef int64_t time_t;
typedef int64_t clock_t;

typedef ssize_t pid_t;

#ifdef _KERNEL
#include <dxgmx/err_or.h>

#define PID_MAX _SSIZE_MAX_
DEFINE_ERR_OR(ino_t);
#endif // DXGMX_KERKEL

#endif // !_DXGMX_POSIX_SYS_TYPES_H
