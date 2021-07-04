
#include<dxgmx/kprintf.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/string.h>
#include<dxgmx/video/tty.h>
#include<stdint.h>
#include<stddef.h>
#include<limits.h>
#include<stdarg.h>

enum PrintfLengthSpecifier
{
    PRINTF_LEN_NONE,
    PRINTF_LEN_hh,
    PRINTF_LEN_h,
    PRINTF_LEN_l,
    PRINTF_LEN_ll,
    PRINTF_LEN_j,
    PRINTF_LEN_z,
    PRINTF_LEN_t,
    PRINTF_LEN_L
};

#define WRITE_CAP INT_MAX

int kprintf(const char *fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    size_t written = vkprintf(fmt, arglist);
    va_end(arglist);
    return written;
}

int vkprintf(const char *fmt, va_list arglist)
{
    size_t written = 0;
    uint8_t length;

    for(; *fmt != '\0'; ++fmt)
    {
        if(written >= WRITE_CAP)
            return written;

        if(*fmt != '%')
        {
            tty_print(fmt, 1);
            ++written;
            continue;
        }
        length = PRINTF_LEN_NONE;

process_fmt:
        switch(*(fmt + 1))
        {
        case '\0':
            return written;

        case 'l':  /* l length specifier */
            if(length == PRINTF_LEN_l)
            {
                ++fmt;
                length = PRINTF_LEN_ll;
                goto process_fmt;
            }
            else if(length != PRINTF_LEN_NONE)
            {
                continue;
            }
            ++fmt;
            length = PRINTF_LEN_l;
            goto process_fmt;
        
        case 'h':  /* h length specifier */
            if(length == PRINTF_LEN_h)
            {
                ++fmt;
                length = PRINTF_LEN_hh;
                goto process_fmt;
            }
            else if(length != PRINTF_LEN_NONE)
            {
                continue;
            }
            ++fmt;
            length = PRINTF_LEN_h;
            goto process_fmt;

        case 'j':  /* j length specifier */
            if(length != PRINTF_LEN_NONE)
                continue;
            ++fmt;
            length = PRINTF_LEN_j;
            goto process_fmt;

        case 'z':  /* z length specifier */
            if(length != PRINTF_LEN_NONE)
                continue;
            ++fmt;
            length = PRINTF_LEN_z;
            goto process_fmt;

        case 't':  /* t length specifier */
            if(length != PRINTF_LEN_NONE)
                continue;
            ++fmt;
            length = PRINTF_LEN_t;
            goto process_fmt;
        
        case 'L': /* L length specifier */
            if(length != PRINTF_LEN_NONE)
                continue;
            ++fmt;
            length = PRINTF_LEN_L;
            goto process_fmt;

        /* int %i %d */
        case 'i':
        case 'd':
        {
            char buf[21] = { 0 };
            switch(length)
            {
            case PRINTF_LEN_l:
            {
                long val = va_arg(arglist, long);
                __ltoa(val, buf, 10);
                break;
            }
            case PRINTF_LEN_ll:
            {
                long long val = va_arg(arglist, long long);
                __lltoa(val, buf, 10);
                break;
            }
            default:
            {
                int val = va_arg(arglist, int);
                __itoa(val, buf, 10);
                break;
            }
            }

            size_t len = __strlen(buf);
            if(written + len > WRITE_CAP)
                return written;
                
            written += len;
            ++fmt;
            
            tty_print(buf, len);
            break;
        }

        /* hexadecimal %x %X */
        case 'x':
        case 'X':
        {
            char buf[21] = { 0 };
            switch(length)
            {
            case PRINTF_LEN_l:
            {
                unsigned long val = va_arg(arglist, unsigned long);
                __ultoa(val, buf, 16);
                break;
            }
            case PRINTF_LEN_ll:
            {
                unsigned long long val = va_arg(arglist, unsigned long long);
                __ulltoa(val, buf, 16);
                break;
            }
            default:
            {
                unsigned val = va_arg(arglist, unsigned);
                __utoa(val, buf, 16);
                break;
            }
            }

            size_t len = __strlen(buf);
            if(written + len > WRITE_CAP)
                return written;
            
            written += len;
            ++fmt;

            tty_print(buf, len);
            break;
        }

        /* string %s */
        case 's':
        {
            char *val = va_arg(arglist, char*);
            size_t len = __strlen(val);
            if(written + len > WRITE_CAP)
                return written;
            written += len;
            ++fmt;

            tty_print(val, len);
            break;
        }

        /* char %c */
        case 'c':
        {
            char val = va_arg(arglist, int);
            ++written;
            ++fmt;

            tty_print(&val, 1);
            break;
        }
        
        }

    }

    return written;
}
