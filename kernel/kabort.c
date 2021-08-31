/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/cpu.h>

void kabort()
{
    cpu_hang();
    __builtin_unreachable();
}
