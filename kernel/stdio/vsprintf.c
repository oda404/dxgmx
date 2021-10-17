
#include<dxgmx/stdio.h>
#include<dxgmx/types.h>
#include<dxgmx/string.h>
#include<dxgmx/todo.h>
#include<dxgmx/math.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/ctype.h>
#include<limits.h>

int vsprintf(char *dest, const char *fmt, va_list arglist)
{
    return vsnprintf(dest, INT_MAX, fmt, arglist);
}
