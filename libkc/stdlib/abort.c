/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/stdlib.h>

void kabort(void)
{
    asm volatile ("1: cli; hlt; jmp 1b");
    __builtin_unreachable();
}
