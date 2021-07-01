
#ifndef __DXGMX_pageframe_H__
#define __DXGMX_pageframe_H__

#include<stdint.h>

void pageframe_alloc_init();
uint64_t pageframe_alloc();
uint32_t pageframe_get_avail_frames_cnt();
void pageframe_free(uint64_t pageframe_base);

#endif // __DXGMX_pageframe_H__
