
#include<dxgmx/types.h>
#include<dxgmx/panic.h>
#include<dxgmx/compiler_attrs.h>

/* TODO: actually randomize it */
ptr __stack_chk_guard = 0xFA7BA115;

_ATTR_NORETURN void __stack_chk_fail()
{
    panic("Stack smashing detected in ring 0 :(");
}
