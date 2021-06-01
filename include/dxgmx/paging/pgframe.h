
#ifndef __DXGMX_PGFRAME_H__
#define __DXGMX_PGFRAME_H__

#include<stdint.h>

void pgframe_alloc_init();
uint64_t pgframe_alloc();
uint32_t pgframe_get_avail_frames_cnt();
void pgframe_free(uint64_t pgframe_base);

#endif // __DXGMX_PGFRAME_H__
