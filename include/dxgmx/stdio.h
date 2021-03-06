/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_STDIO_H
#define _DXGMX_STDIO_H

#include <stdarg.h>
#include <stddef.h>

int sprintf(char* dest, const char* fmt, ...);
int snprintf(char* dest, size_t n, const char* fmt, ...);
int vsprintf(char* dest, const char* fmt, va_list args);
int vsnprintf(char* dest, size_t n, const char* fmt, va_list arglist);

#endif //!_DXGMX_STDIO_H
