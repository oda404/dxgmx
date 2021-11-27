
#include<dxgmx/errno.h>

static int kerrno = 0;

int *_get_errno_addr()
{
    return &kerrno;
}
