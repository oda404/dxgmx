/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stdint.h>

typedef uint16_t mode_t;

typedef uint32_t dev_t;
typedef uint64_t ino_t;
typedef uint32_t nlink_t;

typedef uint32_t uid_t;
typedef uint32_t gid_t;

typedef int64_t off_t;

typedef uint32_t blksize_t;
typedef uint32_t blkcnt_t;

typedef int64_t time_t;
typedef int64_t clock_t;

typedef int64_t loff_t;

#endif // !_SYS_TYPES_H
