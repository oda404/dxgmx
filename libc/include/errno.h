
#ifndef _ERRNO_H
#define _ERRNO_H

int *__get_errno_addr();
#define errno (*__get_errno_addr())

#define ERANGE 1
#define EINVAL 2

#endif // _ERRNO_H