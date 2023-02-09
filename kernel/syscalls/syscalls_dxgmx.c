
#include <dxgmx/kprintf.h>

void syscall_dxgmx_log(const char* fmt, va_list list)
{
    (void)kprintf("userspace: ");
    (void)kvprintf(fmt, list);
}
