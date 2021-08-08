
#include<dxgmx/time.h>
#include<dxgmx/todo.h>

size_t strftime(
    char _ATTR_MAYBE_UNUSED *str, 
    size_t _ATTR_MAYBE_UNUSED maxsize, 
    const char _ATTR_MAYBE_UNUSED *fmt, 
    const struct tm _ATTR_MAYBE_UNUSED *timeptr
)
{
    TODO_FATAL();
    __builtin_unreachable();
}

