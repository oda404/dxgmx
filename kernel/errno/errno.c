/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/errno.h>

static int kerno = 0;

int *__get_errno_addr()
{
    return &kerno;
}