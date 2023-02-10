/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/ctype.h>

int isdigit(int c)
{
    // basic ass ascii check
    return c >= '0' && c <= '9';
}

int isspace(int c)
{
    return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
           c == '\v';
}

int isxdigit(int c)
{
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
        (c >= 'A' && c <= 'F'))
        return 1;

    return 0;
}

int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

int tolower(int c)
{
    return c + 32;
}
