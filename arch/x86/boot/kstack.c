
#include<dxgmx/kstack.h>
#include<dxgmx/attrs.h>
#include<dxgmx/types.h>

_ATTR_ALIGNED(4096) _ATTR_SECTION(".bss") 
static u8 g_kstack[KSTACK_SIZE] = { 0 };

_ATTR_ALWAYS_INLINE ptr kstack_get_top()
{
    return (ptr)&g_kstack[KSTACK_SIZE - 1];
}

_ATTR_ALWAYS_INLINE ptr kstack_get_bot()
{
    return (ptr)&g_kstack[0];
}
