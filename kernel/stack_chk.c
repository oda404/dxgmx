
#include<dxgmx/types.h>
#include<dxgmx/abandon_ship.h>
#include<dxgmx/attrs.h>

/* TODO: actually randomize it */
ptr __stack_chk_guard = 0xFA7BA115;

_ATTR_NORETURN void __stack_chk_fail()
{
    abandon_ship("*** Stack smashing detected in kernel mode :( ***\n");
    __builtin_unreachable();
}
