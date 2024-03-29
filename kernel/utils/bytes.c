/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/string.h>
#include <dxgmx/utils/bytes.h>

float bytes_to_human_readable(u32 bytes, char unit[4])
{
    float fbytes = bytes;
    u8 i = 0;
    while (fbytes >= 1024)
    {
        ++i;
        fbytes /= 1024.f;
    }

    if (unit)
    {
        switch (i)
        {
        case 0:
            strcpy(unit, "B");
            break;
        case 1:
            strcpy(unit, "KiB");
            break;
        case 2:
            strcpy(unit, "MiB");
            break;
        case 3:
            strcpy(unit, "GiB");
            break;
        }
    }
    return fbytes;
}

u64 bytes_align_up64(u64 n, u64 alignment)
{
    if (n % alignment == 0)
        return n;

    return n + (alignment - n % alignment);
}

u64 bytes_align_down64(u64 n, u64 alignment)
{
    return n - (n % alignment);
}
