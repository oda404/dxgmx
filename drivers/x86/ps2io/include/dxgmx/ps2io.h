/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PS2IO_H
#define _DXGMX_PS2IO_H

#include <dxgmx/types.h>

u8 ps2io_read_data_byte_nochk();

int ps2io_set_interrupts(bool state, u8 portn);
int ps2io_set_scanning(bool state, u8 portn);

#endif // !_DXGMX_PS2IO_H
