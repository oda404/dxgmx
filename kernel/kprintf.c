/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kprintf.h>
#include <dxgmx/kstdio.h>
#include <dxgmx/stdio.h>
#include <dxgmx/string.h>

int kprintf(const char* fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    size_t written = kvprintf(fmt, arglist);
    va_end(arglist);
    return written;
}

int kvprintf(const char* fmt, va_list arglist)
{
#define KPRINTF_MAX_BUF_SIZE 256

    char buf[KPRINTF_MAX_BUF_SIZE] = {0};
    vsnprintf(buf, KPRINTF_MAX_BUF_SIZE, fmt, arglist);

    int written;
    written = kstdio_write(buf, strlen(buf));
    return written;
}
