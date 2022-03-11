/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_FALLOC_H
#define _DXGMX_MEM_FALLOC_H

#include <dxgmx/types.h>

/* falloc stands for (page) Frame ALLOCator. */

/**
 * Initializes the page frame allocator.
 * @return 0 on success.
 */
int falloc_init();
/**
 *  Tries to allocate 'n' contiguous page frames.
 * @return 0 on failure. Otherwise the base of the page frame(s).
 */
ptr falloc(size_t n);
/**
 * @return the number of page frames that are currently free.
 */
size_t falloc_get_free_frames_count();
/* Frees 'n' page frames starting from 'base'. */
void ffree(ptr base, size_t n);

#endif // _DXGMX_MEM_FALLOC_H
