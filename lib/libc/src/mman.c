
#include <dxgmx/syscall_defs.h>
#include <errno.h>
#include <sys/mman.h>
#include <syscalls.h>

void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t off)
{
    syscall_ret_t ret =
        syscall6(SYS_MMAP, (syscall_arg_t)addr, len, prot, flags, fd, off);

    if (ret < ERRNO_MAX)
        return (void*)MAP_FAILED; // FIXME: set errno when that is added

    return (void*)ret;
}
