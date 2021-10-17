
#include<dxgmx/stdio.h>

int snprintf(char *dest, size_t n, const char *fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    int written = vsnprintf(dest, n, fmt, arglist);
    va_end(arglist);
    return written;
}
