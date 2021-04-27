/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/stdio.h>
#include<dxgmx/string.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/stdlib.h>
#include<stdarg.h>
#include<stddef.h>
#include<limits.h>

#define PRINTF_FSPEC  '%'
#define PRINTF_CHAR   'c'
#define PRINTF_INT_1  'd'
#define PRINTF_INT_2  'i'
#define PRINTF_FLOAT  'f'
#define PRINTF_STR    's'
#define PRINTF_PTR    'p'
#define PRINTF_NONE   'n'
#define PRINTF_OCT    'o'
#define PRINTF_UINT   'u'
#define PRINTF_HEX_L  'x'
#define PRINTF_HEX_C  'X'

#define PRINTF_WRITE_CAP INT_MAX

int kprintf(const char *fmt, ...)
{
    va_list arg_list;
    va_start(arg_list, fmt);

    size_t written = 0;

    /* 
    this implementation makes a lot of calls
    to tty_print, once malloc is a thing tty_print
    should only be called once
    */

    for(; *fmt != '\0'; ++fmt)
    {
        if(written == PRINTF_WRITE_CAP)
            goto end;

        if(*fmt != PRINTF_FSPEC)
        {
            tty_print(fmt, 1);
            ++written;
            continue;
        }

        /* met a format specifier so we go +1 to find the format */
        switch(*(fmt + 1))
        {
        case '\0':
            tty_print("%", 1);
            ++written;
            goto end;

        case PRINTF_CHAR:
        {
            char val = va_arg(arg_list, int);
            tty_print(&val, 1);
            break;
        }

        case PRINTF_INT_1:
        case PRINTF_INT_2:
        {
            int val = va_arg(arg_list, int);

            char buff[11] = { 0 };
            __itoa(val, buff, 10);

            size_t bufflen = __strlen(buff);

            if(written + bufflen > PRINTF_WRITE_CAP)
                goto end;

            tty_print(buff, bufflen);
            break;
        }

        case PRINTF_HEX_C:
        {
            unsigned int val = va_arg(arg_list, unsigned int);

            val = __abs(val);

            char buff[11] = { 0 };
            __itoa(val, buff, 16);

            size_t bufflen = __strlen(buff);

            if(written + bufflen > PRINTF_WRITE_CAP)
                goto end;

            tty_print(buff, bufflen);
            written += bufflen;
            break;
        }

        case PRINTF_STR:
        {
            char *val = va_arg(arg_list, char*);
            size_t len = __strlen(val);

            if(written + len > PRINTF_WRITE_CAP)
                goto end;
            
            tty_print(val, len);
            written += len;
            break;
        }

        default:
            /* 
            did not match any formats, might still
            be flags, width, precision or length 
            but idc about that yet
            */
            tty_print("%", 1);
            ++written;
            break;
        }
        ++fmt;
    }

end:
    va_end(arg_list);
    return written;
}
