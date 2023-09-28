/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_UTILS_BITMAP_H
#define _DXGMX_UTILS_BITMAP_H

#include <dxgmx/types.h>

typedef struct Bitmap
{
    u8* start;
    size_t size;

    size_t byte_cursor;
    u8 bit_cursor;
} Bitmap;

#define BITMAP_NOT_INIT(bm) (bm.size == 0)

int bitmap_init(size_t units, Bitmap* bm);
ssize_t bitmap_first_n_free_and_mark(size_t n, Bitmap* bm);

#endif // !_DXGMX_UTILS_BITMAP_H
