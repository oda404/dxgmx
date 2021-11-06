
#include<dxgmx/x86/interrupts.h>
#include<dxgmx/x86/pic.h>
#include<dxgmx/compiler_attrs.h>

_ATTR_ALWAYS_INLINE void 
interrupts_disable()
{
    pic8259_set_mask(0xFF, 0);
    pic8259_set_mask(0xFF, 1);
}

_ATTR_ALWAYS_INLINE void 
interrupts_enable()
{
    pic8259_set_mask(0, 0);
    pic8259_set_mask(0, 1);
}
