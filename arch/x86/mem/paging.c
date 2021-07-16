
#include<dxgmx/mem/paging.h>

void paging_enable()
{
    asm volatile(
        "movl $1,   %eax  \n"
        "movl %cr0, %ebx  \n"
        "or   %eax, %ebx  \n"
        "shl  $31,  %eax  \n"
        "or   %eax, %ebx  \n"
        "movl %ebx, %cr0  \n"
    );
}
