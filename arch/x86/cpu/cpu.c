
#include<dxgmx/cpu.h>
#include<dxgmx/gcc/attrs.h>

__ATTR_ALWAYS_INLINE
uint32_t cpu_get_esp()
{
    uint32_t esp;
    asm volatile("movl %%esp, %0" : "=r"(esp));
    return esp;
}
