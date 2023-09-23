
#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H 1

#include <dxgmx/posix/sys/mman.h>
#include <sys/types.h>

void* mmap(void* addr, size_t len, int prot, int flags, int fd, off_t off);

#endif // !_SYS_MMAN_H
