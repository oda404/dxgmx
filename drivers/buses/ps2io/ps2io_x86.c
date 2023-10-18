/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/err_or.h>
#include <dxgmx/errno.h>
#include <dxgmx/ps2io.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/portio.h>

/* Max retry counter when checking the status register if we can read from
 * PS2_DATA or write to PS2_DATA || PS2_CMD */
#define MAX_TRY_COUNT 5

int ps2io_write_byte(u8 data, u8 port)
{
    u8 tries = 0;
    u8 st;

    while ((st = port_inb(PS2_STATUS)) & (1 << 1))
    {
        if (tries++ > MAX_TRY_COUNT)
            return -ETIMEDOUT;
    }

    port_outb(data, port);
    return 0;
}

ERR_OR(u8) ps2io_read_data_byte()
{
    u8 tries = 0;
    u8 st;

    while (!((st = port_inb(PS2_STATUS)) & (1 << 0)))
    {
        if (tries++ > MAX_TRY_COUNT)
            return ERR(u8, -ETIMEDOUT);
    }

    return VALUE(u8, port_inb(PS2_DATA));
}

u8 ps2io_read_data_byte_nochk()
{
    return port_inb(PS2_DATA);
}
