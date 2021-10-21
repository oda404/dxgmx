/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_PAGING_FRAME_H
#define _DXGMX_PAGING_FRAME_H

#include<dxgmx/types.h>

void pgframe_alloc_init();
ptr pgframe_alloc();
u32 pgframe_alloc_get_free_pages();
void pgframe_free(ptr frame);

#endif // _DXGMX_PAGING_FRAME_H
