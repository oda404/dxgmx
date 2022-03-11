/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_UITLS_BYTE_H
#define _DXGMX_UITLS_BYTE_H

#include <dxgmx/types.h>

/* Takes in a number of bytes and transforms them into
the most human readable format, putting the unit used in 'unit'
for example: "B", "KiB", "MiB"*/
float bytes_to_human_readable(u32 bytes, char unit[4]);

#endif //!_DXGMX_UITLS_BYTE_H
