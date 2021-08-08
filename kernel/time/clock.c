/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/time.h>
#include<dxgmx/todo.h>

clock_t clock()
{
    TODO_FATAL();
    __builtin_unreachable();
}
