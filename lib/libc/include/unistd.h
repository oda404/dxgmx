
#ifndef _UNISTD_H
#define _UNISTD_H 1

#include <dxgmx/unistd_defs.h>
#include <stddef.h>
#include <sys/types.h>

ssize_t read(int fd, void* buf, size_t n);

#endif // !_UNISTD_H
