
#include<dxgmx/types.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/compiler_attrs.h>

/* TODO: actually randomize it */
ptr __stack_chk_guard = 0xFA7BA115;

_ATTR_NORETURN void __stack_chk_fail()
{
    abandon_ship("Stack smashing detected in ring 0 :(");
}
