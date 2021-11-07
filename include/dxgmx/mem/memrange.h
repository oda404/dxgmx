
#ifndef _DXGMX_MEM_MEMRANGE_H
#define _DXGMX_MEM_MEMRANGE_H

#include<dxgmx/types.h>

typedef struct 
S_MemRange
{
    u64 base;
    u64 size;
} MemRange;

typedef struct
S_MemRangeTyped
{
    u64 base;
    u64 size;
    u8 type;
} MemRangeTyped;

#endif //!_DXGMX_MEM_MEMRANGE_H
