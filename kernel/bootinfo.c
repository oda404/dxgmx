
#include<dxgmx/bootinfo.h>
#include<dxgmx/string.h>

int bootspec_to_str(uint8_t bootspec, char *out)
{
    switch(bootspec)
    {
    case MULTIBOOT2:
        __strcpy(out, "multiboot2");
        return 0;
    case MULTIBOOT:
        __strcpy(out, "multiboot");
        return 0;
    case STANDALONE:
        __strcpy(out, "standalone");
        return 0;
    default:
        return -1;
    }
}
