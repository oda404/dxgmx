
#ifndef _DXGMX_MEM_MEMRANGE_H
#define _DXGMX_MEM_MEMRANGE_H

#include<dxgmx/types.h>

#define MEM_RANGE_FMT "[mem " PTR_FMT "-" PTR_FMT "]"

typedef struct 
S_MemRange
{
    ptr base;
    size_t size;
} MemRange;

typedef struct
S_MemRangeTyped
{
    ptr base;
    size_t size;
    u8 type;
} MemRangeTyped;

#endif //!_DXGMX_MEM_MEMRANGE_H
