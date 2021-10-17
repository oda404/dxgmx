/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/kprintf.h>
#include<dxgmx/stdio.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/string.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/ctype.h>
#include<dxgmx/todo.h>
#include<dxgmx/math.h>
#include<dxgmx/types.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>
#include<stdarg.h>

int kprintf(const char *fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    size_t written = kvprintf(fmt, arglist);
    va_end(arglist);
    return written;
}

int kvprintf(const char *fmt, va_list arglist)
{
#define KPRINTF_MAX_BUF_SIZE 256

    char buf[KPRINTF_MAX_BUF_SIZE] = { 0 };
    vsnprintf(buf, KPRINTF_MAX_BUF_SIZE, fmt, arglist);

    int written;
    written = tty_print(buf, strlen(buf));
    return written;
}
