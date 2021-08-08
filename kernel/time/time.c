/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/time.h>
#include<dxgmx/todo.h>

time_t time(time_t _ATTR_MAYBE_UNUSED *timer)
{
    TODO_FATAL();
    __builtin_unreachable();
}

