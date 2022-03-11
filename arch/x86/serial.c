/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/compiler_attrs.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/portio.h>
#include <dxgmx/x86/serial.h>

#define BAUDRATE_LO_PORT(x) (x)
#define BAUDRATE_HI_PORT(x) (x + 1)

#define INT_PORT(x) (x + 1)

#define FIFO_PORT(x) (x + 2)
#define FIFO_ENABLE 1
#define FIFO_CLEAR_RECEIVE (1 << 1)
#define FIFO_CLEAR_TRANSMIT (1 << 2)
#define FIFO_INT_TRIGG_LVL_1B 0
#define FIFO_INT_TRIGG_LVL_4B (1 << 6)
#define FIFO_INT_TRIGG_LVL_8B (2 << 6)
#define FIFO_INT_TRIGG_LVL_14B (3 << 6)

#define LINECTRL_PORT(x) (x + 3)
#define LINECTRL_DATABITS(x) (x)
#define LINECTRL_STOPBITS(x) (x << 2)
#define LINECTRL_PARITY(x) (x << 3)
#define LINECTRL_DLAB (1 << 7)

#define MODEMCTRL_PORT(x) (x + 4)
#define MODEMCTRL_DATA_RDY (1)
#define MODEMCTRL_REQ_SEND (1 << 1)
#define MODEMCTRL_OUT1 (1 << 2)
#define MODEMCTRL_OUT2 (1 << 3)
#define MODEMCTRL_LOOPBACK (1 << 4)

bool serial_config_port(const SerialPort* config)
{
    const u16 port = config->port;

    /* Stop all interrups. */
    port_outb(0, INT_PORT(port)); // stop interrupts

    /* Set DLAB. */
    port_outb(LINECTRL_DLAB, LINECTRL_PORT(port));

    /* set baudrate. */
    port_outb(config->baudrate & 0xFF, BAUDRATE_LO_PORT(port));
    port_outb(config->baudrate >> 8, BAUDRATE_HI_PORT(port));

    /* Set bit pattern. */
    port_outb(
        LINECTRL_DATABITS(config->databits) |
            LINECTRL_STOPBITS(config->stopbits) |
            LINECTRL_PARITY(config->parity),
        LINECTRL_PORT(port));

    /* Set FIFO. */
    port_outb(
        FIFO_ENABLE | FIFO_CLEAR_RECEIVE | FIFO_CLEAR_TRANSMIT |
            FIFO_INT_TRIGG_LVL_14B,
        FIFO_PORT(port));

    /* Set port in loopback mode to test it. */
    port_outb(
        MODEMCTRL_REQ_SEND | MODEMCTRL_OUT1 | MODEMCTRL_OUT2 |
            MODEMCTRL_LOOPBACK,
        MODEMCTRL_PORT(port));

    port_outb(0x69, port);
    if (port_inb(port) != 0x69)
        return false;

    /* Port is good, exit loopback mode. */
    port_outb(
        MODEMCTRL_DATA_RDY | MODEMCTRL_REQ_SEND | MODEMCTRL_OUT1 |
            MODEMCTRL_OUT2,
        MODEMCTRL_PORT(port));

    return true;
}

void serial_write(u8 byte, u16 port)
{
    port_outb(byte, port);
}
