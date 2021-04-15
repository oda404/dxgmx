
#ifndef __DXGMX_KERRNO_H__
#define __DXGMX_KERRNO_H__

#define ERANGE 1
#define EINVAL 2

int *__get_errno_addr();
/* 
 * errno is the one thing that is not prefixed with k
 * because of later libc usage
*/
#define errno (*__get_errno_addr())

#endif // __DXGMX_KERRNO_H__
