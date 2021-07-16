
#include<dxgmx/kprintf.h>
#include<dxgmx/stdlib.h>
#include<dxgmx/string.h>
#include<dxgmx/video/tty.h>
#include<dxgmx/ctype.h>
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

enum PrintfFlags
{
    PRINTF_FLAG_NONE           = 0x0,
    PRINTF_FLAG_LEFT_JUSTIFY   = 0x1,
    PRINTF_FLAG_FORCE_SIGN     = 0x2,
    PRINTF_FLAG_WHITESPACE     = 0x4,
    PRINTF_FLAG_HASHTAG        = 0x8,
    PRITNF_FLAG_PAD_ZERO       = 0x10
};

enum PrintfParseStatus
{
    PRINTF_PARSE_CONTINUE,
    PRINTF_PARSE_INVALID,
    PRINTF_PARSE_DONE
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

static int printf_parse_flags(char c, uint8_t *flags)
{
    switch(c)
    {
    case '-':
        *flags |= PRINTF_FLAG_LEFT_JUSTIFY;
        return PRINTF_PARSE_CONTINUE;
    case '+':
        *flags  |= PRINTF_FLAG_FORCE_SIGN;
        return PRINTF_PARSE_CONTINUE;
    case ' ':
        *flags  |= PRINTF_FLAG_WHITESPACE;
        return PRINTF_PARSE_CONTINUE;
    case '#':
        *flags  |= PRINTF_FLAG_HASHTAG;
        return PRINTF_PARSE_CONTINUE;
    case '0':
        *flags  |= PRITNF_FLAG_PAD_ZERO;
        return PRINTF_PARSE_CONTINUE;
    default:
        return PRINTF_PARSE_DONE;
    }
}

static int printf_parse_width(char c, uint16_t *width)
{
    if(isdigit(c))
    {
        *width = *width * 10 + (c - '0');
        return PRINTF_PARSE_CONTINUE;
    }

    return PRINTF_PARSE_DONE;
}

static int printf_parse_length(char c, uint8_t *length)
{
    switch(c)
    {
    case 'l':
        if(*length == PRINTF_LEN_l)
        {
            *length = PRINTF_LEN_ll;
            return PRINTF_PARSE_CONTINUE;
        }
        else if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        
        *length = PRINTF_LEN_l;
        return PRINTF_PARSE_CONTINUE;
    
    case 'h':
        if(*length == PRINTF_LEN_h)
        {
            *length = PRINTF_LEN_hh;
            return PRINTF_PARSE_CONTINUE;
        }
        else if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        
        *length = PRINTF_LEN_h;
        return PRINTF_PARSE_CONTINUE;

    case 'j':
        if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_j;
        return PRINTF_PARSE_CONTINUE;

    case 'z':
        if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_z;
        return PRINTF_PARSE_CONTINUE;

    case 't':
        if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_t;
        return PRINTF_PARSE_CONTINUE;
    
    case 'L':
        if(*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_L;
        return PRINTF_PARSE_CONTINUE;

    default:
        return PRINTF_PARSE_DONE;
    }
}

static size_t printf_apply_flags_and_width(
    char *buf,
    size_t bufmax,
    uint8_t flags,
    size_t width
)
{
    if(width > bufmax)
        width = bufmax;

    size_t buflen = strlen(buf);

    if(flags & PRINTF_FLAG_LEFT_JUSTIFY)
    {
        //TODO
    }

    if(flags & PRINTF_FLAG_FORCE_SIGN)
    {
        //TODO
    }

    if(flags & PRINTF_FLAG_WHITESPACE)
    {
        //TODO
    }

    if(flags & PRINTF_FLAG_HASHTAG)
    {
        //TODO
    }

    if(width > buflen)
    {
        const char c = (flags & PRITNF_FLAG_PAD_ZERO) ? '0' : ' ';
        for(size_t i = 0; i < width - buflen; ++i)
        {
            for(size_t k = buflen + i; k > 0; --k)
                buf[k] = buf[k - 1];
            buf[i] = c;
        }
    }

    return strlen(buf);
}

int vkprintf(const char *fmt, va_list arglist)
{
    size_t written = 0;
    uint8_t length;
    uint8_t flags;
    uint16_t width;

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
        flags  = PRINTF_FLAG_NONE;
        width = 0;

        while(printf_parse_flags(*(fmt + 1), &flags) == PRINTF_PARSE_CONTINUE)
            ++fmt;

        while(printf_parse_width(*(fmt + 1), &width) == PRINTF_PARSE_CONTINUE)
            ++fmt;
        
reprocess_length:
        switch(printf_parse_length(*(fmt + 1), &length))
        {
        case PRINTF_PARSE_INVALID:
            continue;
        case PRINTF_PARSE_CONTINUE:
            ++fmt;
            goto reprocess_length;
        case PRINTF_PARSE_DONE:
            break;
        }

        // specifier
        switch(*(fmt + 1))
        {
        /* int %i %d */
        case 'i':
        case 'd':
        {
            char buf[21] = { 0 };
            switch(length)
            {
            case PRINTF_LEN_l:
            {
                ltoa(va_arg(arglist, long), buf, 10);
                break;
            }
            case PRINTF_LEN_ll:
            {
                lltoa(va_arg(arglist, long long), buf, 10);
                break;
            }
            default:
            {
                itoa(va_arg(arglist, int), buf, 10);
                break;
            }
            }
            
            size_t len = printf_apply_flags_and_width(buf, 21, flags, width);

            if(written + len > WRITE_CAP)
            {
                tty_print(buf, written + len - WRITE_CAP);
                return WRITE_CAP;
            }
                
            written += len;
            
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
                ultoa(va_arg(arglist, unsigned long), buf, 16);
                break;
            }
            case PRINTF_LEN_ll:
            {
                ulltoa(va_arg(arglist, unsigned long long), buf, 16);
                break;
            }
            default:
            {
                utoa(va_arg(arglist, unsigned), buf, 16);
                break;
            }
            }

            size_t len = printf_apply_flags_and_width(buf, 21, flags, width);

            if(written + len > WRITE_CAP)
            {
                tty_print(buf, written + len - WRITE_CAP);
                return WRITE_CAP;
            }

            written += len;

            tty_print(buf, len);
            break;
        }

        /* string %s */
        case 's':
        {
            const char *val = va_arg(arglist, const char*);
            size_t len = strlen(val);

            if(written + len > WRITE_CAP)
            {
                tty_print(val, written + len - WRITE_CAP);
                return WRITE_CAP;
            }

            written += len;

            tty_print(val, len);
            break;
        }

        /* char %c */
        case 'c':
        {
            char val = va_arg(arglist, int);
            ++written;

            tty_print(&val, 1);
            break;
        }
        
        }
        ++fmt;
    }

    return written;
}
