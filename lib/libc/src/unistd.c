
#include <dxgmx/syscall_defs.h>
#include <syscalls.h>
#include <unistd.h>

ssize_t read(int fd, void* buf, size_t n)
{
    return syscall3(SYS_READ, fd, (syscall_arg_t)buf, n);
}

ssize_t write(int fd, const void* buf, size_t n)
{
    return syscall3(SYS_WRITE, fd, (syscall_arg_t)buf, n);
}
