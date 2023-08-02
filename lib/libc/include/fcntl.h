/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _FCNTL_H
#define _FCNTL_H 1

#include <dxgmx/fcntl_defs.h>
#include <sys/types.h>

int open(const char* path, int flags, mode_t mode);

#endif // !_FCNTL_H
