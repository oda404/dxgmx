
#include <dxgmx/syscall_defs.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <syscalls.h>

int ioctl(int fd, int req, ...)
{
    va_list list;
    va_start(list, req);
    size_t data = va_arg(list, size_t);
    va_end(list);

    return syscall3(SYS_IOCTL, fd, req, data);
}
