
#include<dxgmx/cpu.h>

uint32_t cpu_get_cr2()
{
    uint32_t ret;
    asm volatile("movl %%cr2, %0": "=a"(ret));
    return ret;
}
