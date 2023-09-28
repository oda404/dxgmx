/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/math.h>
#include <dxgmx/utils/bitmap.h>

static int bitmap_mark_bitmap(size_t byte, u8 bit, size_t n, Bitmap* bm)
{
    for (size_t i = byte; i < bm->size; ++i)
    {
        for (u8 k = bit; k < 8; ++k)
        {
            bm->start[i] |= (1 << k);
            if (--n == 0)
                return 0;
        }
    }

    return -1;
}

int bitmap_init(size_t units, Bitmap* bm)
{
    if (!units)
        return -EINVAL;

    const size_t size = ceil(units / 8.0);
    bm->start = kcalloc(size);
    if (!bm->start)
        return -ENOMEM;

    bm->size = size;
    bm->byte_cursor = 0;
    bm->bit_cursor = 0;
    return 0;
}

ssize_t bitmap_first_n_free_and_mark(size_t n, Bitmap* bm)
{
    size_t found = 0;
    size_t byte_start = 0;
    size_t bit_start = 0;

    /* There's a LOT of room for improvement here, but it works for now */
    for (; bm->byte_cursor < bm->size; ++bm->byte_cursor)
    {
        u8 val = bm->start[bm->byte_cursor];
        for (; bm->bit_cursor < 8; ++bm->bit_cursor)
        {
            if (!(val & (1 << bm->bit_cursor)))
            {
                if (!found)
                {
                    byte_start = bm->byte_cursor;
                    bit_start = bm->bit_cursor;
                }

                if (++found != n)
                    continue;

                ASSERT(bitmap_mark_bitmap(byte_start, bit_start, n, bm) == 0);
                if (++bm->bit_cursor == 8)
                {
                    if (++bm->byte_cursor == bm->size)
                        bm->byte_cursor = 0;

                    bm->bit_cursor = 0;
                }
                return byte_start * 8 + bit_start;
            }
            else
            {
                found = 0;
            }
        }

        if (bm->bit_cursor == 8)
            bm->bit_cursor = 0;
    }

    return -ENOMEM;
}
