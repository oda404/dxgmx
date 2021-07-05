/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#ifndef _DXGMX_ERRNO_H
#define _DXGMX_KCONFIG_H

#define ERANGE 1
#define EINVAL 2

int *__get_errno_addr();
/* 
 * errno is the one thing that is not prefixed with k
 * because of later libc usage
*/
#define errno (*__get_errno_addr())

#endif // _DXGMX_KCONFIG_H
