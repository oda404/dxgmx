/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_PS2IO_H
#define _DXGMX_PS2IO_H

#include <dxgmx/generated/kconfig.h>
#include <dxgmx/types.h>

#ifdef CONFIG_X86
#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD 0x64
#endif

ERR_OR(u8) ps2io_read_data_byte();
int ps2io_write_byte(u8 data, u8 port);
u8 ps2io_read_data_byte_nochk();

int ps2io_set_interrupts(bool state, u8 portn);
int ps2io_set_scanning(bool state, u8 portn);

#endif // !_DXGMX_PS2IO_H
