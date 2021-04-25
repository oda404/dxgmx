
#ifndef __DXGMX_BOOTINFO_H__
#define __DXGMX_BOOTINFO_H__

typedef enum S_BootSpec
{
    MULTIBOOT2,
    MULTIBOOT,
    STANDALONE
} BootSpec;

#include<stdint.h>

typedef struct S_BootInfoMemMapEntry
{
    uint64_t base;
    uint64_t size;
    uint32_t type;
} BootInfoMemMapEntry;

typedef struct S_BootInfoMemMap
{
    uint32_t size;
    BootInfoMemMapEntry *entries;
} BootInfoMemMap;

typedef struct S_BootInfo
{
    BootInfoMemMap mmap;
} BootInfo;

/* returns 0 and sets out, if a valid bootspec was provided, 
 * returns -1 and doesn t touch out, otherwise. 
 */
int bootspec_to_str(uint8_t bootspec, char *out);

#endif // __DXGMX_BOOTINFO_H__
