
#include<dxgmx/purgatory.h>

void purgatory_enter()
{
    asm("1: cli; hlt; jmp 1");
}
