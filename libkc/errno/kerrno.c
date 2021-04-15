
#include<dxgmx/kerrno.h>

static int kerno = 0;

int *__get_errno_addr()
{
    return &kerno;
}
