/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/time.h>
#include<dxgmx/todo.h>

double difftime(
    time_t _ATTR_MAYBE_UNUSED time1, 
    time_t _ATTR_MAYBE_UNUSED time2
)
{
    TODO_FATAL();
    __builtin_unreachable();
}
