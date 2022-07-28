/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_PAGING_H
#define _DXGMX_MEM_PAGING_H

#include <dxgmx/mem/pagesize.h>
#include <dxgmx/types.h>

typedef struct S_PagingStruct
{
    ptr vaddr;
} PagingStruct;

#endif // !_DXGMX_MEM_PAGING_H
