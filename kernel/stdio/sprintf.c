
#include<dxgmx/stdio.h>
#include<dxgmx/types.h>

int sprintf(char *dest, const char *fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    size_t written = vsprintf(dest, fmt, arglist);
    va_end(arglist);
    return written;
}
