/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_MEM_FALLOC_H
#define _DXGMX_MEM_FALLOC_H

#include <dxgmx/types.h>

/* falloc stands for (page) Frame ALLOCator. */

/**
 * Initializes the page frame allocator.
 * Returns 0 on success, negative on error.
 */
int falloc_init();

/**
 * Tries to allocate one 'user' pageframe.
 * This pageframe should not be used for the kernel heap, as it's not guaranteed
 * to be mapped in a higher-half fashion in the kernel page table. Kheap page
 * frames need to be allocated 'manually' using falloc_one_at() where we can
 * request frames that we can map in a higher half function overselvers.
 * Returns the page frame physical address on success, 0 on failure.
 */

ptr falloc_one();

/* Returns true if the pageframe at starting at 'base' is not
 * allocated.
 */
bool falloc_is_frame_available(ptr base);

/**
 * Tries to allocate the pageframe starting at physical address 'base'.
 * Returns 0 on success,
 * -ENOMEM if the requested pageframe is already allocated.
 */
int falloc_one_at(ptr base);

/**
 * Returns the number of page frames that are currently free.
 */
size_t falloc_get_free_frames_count();

/* Free the pageframe starting at 'base'. */
void ffree_one(ptr base);

#endif // _DXGMX_MEM_FALLOC_H
