
#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <dxgmx/posix/unistd.h>
#include <stddef.h>
#include <sys/types.h>

ssize_t read(int fd, void* buf, size_t n);
ssize_t write(int fd, const void* buf, size_t n);

#endif // !_UNISTD_H
