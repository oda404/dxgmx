
#include<stdlib.h>

void *malloc(size_t size)
{
#ifdef __LIBC_FREESTANDING__ /* kspace */
    size = 2;
#else /* user space */

#endif // __LIBC_FRESTANDING__
}

void free(void *ptr)
{
#ifdef __LIBC_FREESTANDING__ /* kmode */
    ptr = NULL;
#else /* user space */

#endif // __LIBC_FRESTANDING__
}
