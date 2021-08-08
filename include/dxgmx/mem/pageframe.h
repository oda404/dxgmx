/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#ifndef _DXGMX_PAGING_FRAME_H
#define _DXGMX_PAGING_FRAME_H

#include<stdint.h>

void pageframe_alloc_init();
uint64_t pageframe_alloc();
uint32_t pageframe_get_avail_frames_cnt();
void pageframe_free(uint64_t pageframe_base);

#endif // _DXGMX_PAGING_FRAME_H
