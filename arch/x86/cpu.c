
#include<dxgmx/cpu.h>

uint32_t cpu_get_cr2()
{
    uint32_t ret;
    asm volatile("movl %%cr2, %0": "=a"(ret));
    return ret;
}

uint32_t cpu_get_cr0()
{
    uint32_t ret;
    asm volatile("movl %%cr0, %0": "=a"(ret));
    return ret;
}

void cpu_set_cr0(uint32_t val)
{
    asm volatile("movl %0, %%cr0": :"a"(val));
}
