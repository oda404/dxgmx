
#include<dxgmx/purgatory.h>

void purgatory_enter()
{
    asm("1: cli; hlt; jmp 1");
}

void purgatory_enter_msg(const char *last_msg)
{
    asm("1: cli; hlt; jmp 1");
}
