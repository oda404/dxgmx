
#include<stdio.h>
#include<stdarg.h>
#include<dxgmx/video/tty.h>
#include<stddef.h>
#include<stdlib.h>
#include<string.h>

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

int printf(const char *fmt, ...)
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
        size_t maxrem = __SIZE_MAX__ - written;
        if(maxrem == 0)
        {
            return -1;
        }

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
            break;

        case PRINTF_CHAR:
        {
            char val = va_arg(arg_list, int);
            tty_print(&val, 1);
            ++written;
            ++fmt; // increment so we jump over the format
            break;
        }

        case PRINTF_STR:
        {
            char *val = va_arg(arg_list, char*);
            size_t len = strlen(val);
            tty_print(val, len);
            written += len;
            ++fmt; // increment so we jump over the format
            break;
        }

        case PRINTF_INT_1:
        case PRINTF_INT_2:
        {
            int val = va_arg(arg_list, int);
            if(val == 0)
            {
                tty_print("0", 1);
                ++written;
                ++fmt;
                break;
            }
            if(val < 0)
            {
                tty_print("-", 1);
                val = abs(val);
                ++written;
            }

            /* reverse int */
            int tmp = 0;
            size_t size = 0;
            while(val > 0) 
            {
                tmp = tmp * 10 + val % 10;
                val /= 10;
                ++size;
            }

            while(size--)
            {
                const char c = tmp % 10 + '0';
                tty_print(&c, 1);
                ++written;
                tmp /= 10;
            }

            ++fmt; // increment so we jump over the format
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
            ++fmt; // increment so we jump over the format
            break;
        }
    }

    va_end(arg_list);
    return written;
}
