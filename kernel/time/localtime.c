
#include<dxgmx/time.h>
#include<dxgmx/todo.h>

struct tm *localtime(const time_t _ATTR_MAYBE_UNUSED *timer)
{
    TODO_FATAL();
    __builtin_unreachable();
}
