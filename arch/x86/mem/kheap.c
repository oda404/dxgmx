/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/mem/kheap.h>
#include <dxgmx/string.h>

/* Optimistically I'd want to get rid of the g_kheap array and
initialize a kernel heap completely dynamically, but doing it that
way means we have to reserve or somehow allocate some extra paging
structures which is pretty hard considering we don't have a heap yet :)
Doing it this way means we have a bulkier kernel image but it's all there
for us to use. Until further notice this is how it works.
The reason for this file being architecture specific is because as I mentioned
above, I feel we can do better, and doing so will most likely
require architecture specific stuff.
*/

static _ATTR_ALIGNED(4096) u8 g_kheap[KERNEL_HEAP_SIZE];

_INIT bool kheap_init()
{
    memset(g_kheap, 0, KERNEL_HEAP_SIZE);
    return true;
}

size_t kheap_get_size()
{
    return KERNEL_HEAP_SIZE;
}

ptr kheap_get_start_vaddr()
{
    return (ptr)g_kheap;
}
