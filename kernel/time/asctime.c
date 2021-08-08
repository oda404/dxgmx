/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/time.h>
#include<dxgmx/todo.h>

char *asctime(const struct tm _ATTR_MAYBE_UNUSED *timeptr)
{
    TODO_FATAL();
    __builtin_unreachable();
}
