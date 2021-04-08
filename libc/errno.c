
#include<errno.h>

int kerrno = 0;

int *__get_errno_addr()
{
#ifdef __LIBC_FREESTANDING__
    return &kerrno;
#endif // #ifdef __LIBC_FREESTANDING__
}
