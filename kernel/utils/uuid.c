/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/errno.h>
#include <dxgmx/stdio.h>
#include <dxgmx/todo.h>
#include <dxgmx/utils/uuid.h>

int uuid_format(u32 a, u16 b, u16 c, u16 d, u64 e, char* dest)
{
    if (!dest)
        return -EINVAL;

    return snprintf(dest, UUID_LENGTH + 1, UUID_FMT, a, b, c, d, e);
}

int uuid_gen(char* dest)
{
    (void)dest;
    TODO_FATAL();

    return -1;
}
