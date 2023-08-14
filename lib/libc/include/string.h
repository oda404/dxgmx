/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _STRING_H
#define _STRING_H

#include <sys/types.h>

void* memset(void* s, int c, size_t n) __attribute__((nonnull(1)));

#endif // !_STRING_H
