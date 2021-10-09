
#include<dxgmx/kstack.h>
#include<dxgmx/attrs.h>
#include<dxgmx/types.h>

_ATTR_ALIGNED(4096) _ATTR_SECTION(".bss") 
static u8 _kstack[KSTACK_SIZE] = { 0 };

const ptr _kstack_top = (ptr)&_kstack[KSTACK_SIZE - 1];
const ptr _kstack_bot = (ptr)&_kstack[0];
