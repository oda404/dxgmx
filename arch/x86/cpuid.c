
#include<dxgmx/x86/cpuid.h>
#include<dxgmx/string.h>
#include<stdint.h>

#define CPUID_VENDOR_STR 0
#define CPUID_MAX_EAX    0

int cpuid_is_avail()
{       
    int ret;
    asm volatile(
        "pushfl                                                                                   \n"
        "pushfl                                                                                   \n"
        "xorl   $0x200000, (%%esp)  # flip bit 21                                                 \n"
        "popfl                      # pop modified eflags                                         \n"
        "pushfl                     # push 'modified' eflags                                      \n"
        "popl   %%eax                                                                             \n"
        "xor    (%%esp),   %%eax    # xor 'modified' and original together to see if they changed \n"
        "and    $0x200000, %%eax                                                                  \n"
        "shr    $21,       %%eax                                                                  \n"
        "popfl                      # restore original eflags                                     \n"
        : "=a"(ret)
    );
    return ret;
}

void cpuid_get_vendor_str(char *str)
{
    uint32_t dead;
    uint32_t *b = (uint32_t *)&str[0];
    uint32_t *c = (uint32_t *)&str[4];
    uint32_t *d = (uint32_t *)&str[8];

    CPUID(CPUID_VENDOR_STR, dead, *b, *d, *c);
    str[12] = '\0';
}

uint32_t cpuid_get_eax_max()
{
    uint32_t dead;
    uint32_t ret;

    CPUID(CPUID_MAX_EAX, ret, dead, dead, dead);

    return ret;
}
