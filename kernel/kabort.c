/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/kabort.h>

void kabort()
{
    asm volatile ("1: cli; hlt; jmp 1b");
    __builtin_unreachable();
}
