/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/ctype.h>
#include <dxgmx/math.h>
#include <dxgmx/stdio.h>
#include <dxgmx/stdlib.h>
#include <dxgmx/string.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <limits.h>

int vsprintf(char* dest, const char* fmt, va_list arglist)
{
    return vsnprintf(dest, INT_MAX, fmt, arglist);
}

int snprintf(char* dest, size_t n, const char* fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    int written = vsnprintf(dest, n, fmt, arglist);
    va_end(arglist);
    return written;
}

int sprintf(char* dest, const char* fmt, ...)
{
    va_list arglist;
    va_start(arglist, fmt);
    size_t written = vsprintf(dest, fmt, arglist);
    va_end(arglist);
    return written;
}

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
    PRINTF_FLAG_NONE = 0x0,
    PRINTF_FLAG_LEFT_JUSTIFY = 0x1,
    PRINTF_FLAG_FORCE_SIGN = 0x2,
    PRINTF_FLAG_WHITESPACE = 0x4,
    PRINTF_FLAG_HASHTAG = 0x8,
    PRITNF_FLAG_PAD_ZERO = 0x10
};

enum PrintfParseStatus
{
    PRINTF_PARSE_CONTINUE,
    PRINTF_PARSE_INVALID,
    PRINTF_PARSE_DONE
};

static int printf_parse_flags(char c, u8* flags)
{
    switch (c)
    {
    case '-':
        *flags |= PRINTF_FLAG_LEFT_JUSTIFY;
        return PRINTF_PARSE_CONTINUE;
    case '+':
        *flags |= PRINTF_FLAG_FORCE_SIGN;
        return PRINTF_PARSE_CONTINUE;
    case ' ':
        *flags |= PRINTF_FLAG_WHITESPACE;
        return PRINTF_PARSE_CONTINUE;
    case '#':
        *flags |= PRINTF_FLAG_HASHTAG;
        return PRINTF_PARSE_CONTINUE;
    case '0':
        *flags |= PRITNF_FLAG_PAD_ZERO;
        return PRINTF_PARSE_CONTINUE;
    default:
        return PRINTF_PARSE_DONE;
    }
}

static int printf_parse_length(char c, u8* length)
{
    switch (c)
    {
    case 'l':
        if (*length == PRINTF_LEN_l)
        {
            *length = PRINTF_LEN_ll;
            return PRINTF_PARSE_CONTINUE;
        }
        else if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;

        *length = PRINTF_LEN_l;
        return PRINTF_PARSE_CONTINUE;

    case 'h':
        if (*length == PRINTF_LEN_h)
        {
            *length = PRINTF_LEN_hh;
            return PRINTF_PARSE_CONTINUE;
        }
        else if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;

        *length = PRINTF_LEN_h;
        return PRINTF_PARSE_CONTINUE;

    case 'j':
        if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_j;
        return PRINTF_PARSE_CONTINUE;

    case 'z':
        if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_z;
        return PRINTF_PARSE_CONTINUE;

    case 't':
        if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_t;
        return PRINTF_PARSE_CONTINUE;

    case 'L':
        if (*length != PRINTF_LEN_NONE)
            return PRINTF_PARSE_INVALID;
        *length = PRINTF_LEN_L;
        return PRINTF_PARSE_CONTINUE;

    default:
        return PRINTF_PARSE_DONE;
    }
}

static size_t
printf_apply_flags_and_width(char* buf, size_t bufmax, u8 flags, size_t width)
{
    if (width > bufmax)
        width = bufmax;

    size_t buflen = strlen(buf);

    if (flags & PRINTF_FLAG_LEFT_JUSTIFY)
    {
        // TODO
    }

    if (flags & PRINTF_FLAG_FORCE_SIGN)
    {
        // TODO
    }

    if (flags & PRINTF_FLAG_WHITESPACE)
    {
        // TODO
    }

    if (flags & PRINTF_FLAG_HASHTAG)
    {
        // TODO
    }

    if (width > buflen)
    {
        const char c = (flags & PRITNF_FLAG_PAD_ZERO) ? '0' : ' ';
        for (size_t i = 0; i < width - buflen; ++i)
        {
            for (size_t k = buflen + i; k > 0; --k)
                buf[k] = buf[k - 1];
            buf[i] = c;
        }
    }

    return strlen(buf);
}

int vsnprintf(char* dest, size_t n, const char* fmt, va_list arglist)
{
    dest[0] = '\0';

    size_t written = 0;
    u8 length;
    u8 flags;
    u16 width;
    i64 precision;

    for (; *fmt != '\0'; ++fmt)
    {
        if (written >= n - 1)
            return written;

        if (*fmt != '%')
        {
            strncat(dest, fmt, 1);
            ++written;
            continue;
        }
        // jump over the '%'
        ++fmt;
        length = PRINTF_LEN_NONE;
        flags = PRINTF_FLAG_NONE;
        width = 0;
        precision = -1;

        while (printf_parse_flags(*fmt, &flags) == PRINTF_PARSE_CONTINUE)
            ++fmt;

        /* width */
        if (*fmt == '*')
        {
            width = va_arg(arglist, int);
            ++fmt;
        }
        else
        {
            while (isdigit(*fmt))
            {
                width = width * 10 + (*fmt - '0');
                ++fmt;
            }
        }

        /* precision */
        if (*fmt == '.')
        {
            ++fmt;
            if (*fmt == '*')
            {
                precision = va_arg(arglist, size_t);
            }
            else
            {
                precision = 0;
                while (isdigit(*fmt))
                {
                    precision = precision * 10 + (*fmt++ - '0');
                }
            }
        }

    reprocess_length:
        switch (printf_parse_length(*fmt, &length))
        {
        case PRINTF_PARSE_INVALID:
            continue;
        case PRINTF_PARSE_CONTINUE:
            ++fmt;
            goto reprocess_length;
        case PRINTF_PARSE_DONE:
            break;
        }

        char tmpbuf[50] = {0};
        char* outbuf = tmpbuf;
        size_t outbuf_len = 0;

        // specifier
        switch (*fmt)
        {
        /* int %i %d */
        case 'i':
        case 'd':
        {
            switch (length)
            {
            case PRINTF_LEN_l:
            {
                ltoa(va_arg(arglist, long), tmpbuf, 10);
                break;
            }
            case PRINTF_LEN_ll:
            {
                lltoa(va_arg(arglist, long long), tmpbuf, 10);
                break;
            }
            default:
            {
                itoa(va_arg(arglist, int), tmpbuf, 10);
                break;
            }
            }

            outbuf_len = printf_apply_flags_and_width(tmpbuf, 21, flags, width);
            outbuf = tmpbuf;
            break;
        }

        /* hexadecimal %x %X */
        case 'x':
        case 'X':
        {
            switch (length)
            {
            case PRINTF_LEN_l:
            {
                ultoa(va_arg(arglist, unsigned long), tmpbuf, 16);
                break;
            }
            case PRINTF_LEN_ll:
            {
                ulltoa(va_arg(arglist, unsigned long long), tmpbuf, 16);
                break;
            }
            default:
            {
                utoa(va_arg(arglist, unsigned), tmpbuf, 16);
                break;
            }
            }

            outbuf_len = printf_apply_flags_and_width(tmpbuf, 21, flags, width);
            outbuf = tmpbuf;
            break;
        }

        case 'p':
        {
            ultoa(va_arg(arglist, ptr), tmpbuf, 16);
            flags = PRITNF_FLAG_PAD_ZERO;
            width = sizeof(ptr) == 4 ? 8 : 16;

            outbuf_len = printf_apply_flags_and_width(tmpbuf, 21, flags, width);
            outbuf = tmpbuf;
            break;
        }

        case 'u':
        {
            switch (length)
            {
            case PRINTF_LEN_hh:
                utoa(va_arg(arglist, unsigned int), tmpbuf, 10);
                break;
            case PRINTF_LEN_h:
                utoa(va_arg(arglist, unsigned int), tmpbuf, 10);
                break;
            case PRINTF_LEN_l:
                ultoa(va_arg(arglist, unsigned long int), tmpbuf, 10);
                break;
            case PRINTF_LEN_ll:
                ulltoa(va_arg(arglist, unsigned long long int), tmpbuf, 10);
                break;
            case PRINTF_LEN_j:
                TODO();
                break;
            case PRINTF_LEN_t:
                TODO();
                break;
            case PRINTF_LEN_L:
                break;
            case PRINTF_LEN_z:
            default:
                utoa(va_arg(arglist, unsigned int), tmpbuf, 10);
                break;
            }

            outbuf_len = printf_apply_flags_and_width(tmpbuf, 21, flags, width);
            outbuf = tmpbuf;
            break;
        }

        case 'F': /* what does %F exactly do ?? */
        case 'f':
        {
            long double val = length == PRINTF_LEN_L
                                  ? va_arg(arglist, long double)
                                  : (long double)va_arg(arglist, double);

            long double whole, frac;
            frac = modfl(val, &whole);

            lltoa((long long)whole, tmpbuf, 10);

            size_t digits = precision != -1 ? precision : 6;
            if (digits > 0)
            {
                /* Don't try to write more than 20 digits. */
                if (digits > __LDBL_DECIMAL_DIG__)
                    digits = __LDBL_DECIMAL_DIG__;

                if (frac < 0)
                    frac *= -1;

                strcat(tmpbuf, ".");
                size_t idx = strlen(tmpbuf);
                while (digits--)
                {
                    frac *= 10;
                    lltoa((long long)(frac), tmpbuf + idx++, 10);
                    frac -= (long long)frac;
                }
            }

            outbuf_len = printf_apply_flags_and_width(
                tmpbuf, __LDBL_DIG__ + __LDBL_DECIMAL_DIG__ + 3, flags, width);
            outbuf = tmpbuf;

            break;
        }

        /* string %s */
        case 's':
        {
            outbuf = va_arg(arglist, char*);
            if (precision != -1 && (size_t)precision <= strlen(outbuf))
                outbuf_len = precision;
            else
                outbuf_len = strlen(outbuf);

            break;
        }

        /* char %c */
        case 'c':
        {
            *outbuf = va_arg(arglist, int);
            outbuf_len = 1;
            break;
        }
        }

        if (written + outbuf_len > n - 1)
        {
            strncat(dest, outbuf, outbuf_len - (outbuf_len - n + 1));
            return written + outbuf_len;
        }

        written += outbuf_len;
        strncat(dest, outbuf, outbuf_len);
    }

    return written;
}
