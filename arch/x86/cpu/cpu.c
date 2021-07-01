
#include<dxgmx/cpu.h>
#include<dxgmx/attrs.h>

_ATTR_ALWAYS_INLINE
uint32_t cpu_get_esp()
{
    uint32_t esp;
    asm volatile("movl %%esp, %0" : "=r"(esp));
    return esp;
}
