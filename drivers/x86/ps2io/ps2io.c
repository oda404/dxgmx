/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/assert.h>
#include <dxgmx/errno.h>
#include <dxgmx/klog.h>
#include <dxgmx/module.h>
#include <dxgmx/ps2io.h>
#include <dxgmx/serialio.h>
#include <dxgmx/timeout.h>
#include <dxgmx/todo.h>
#include <dxgmx/types.h>
#include <dxgmx/x86/portio.h>

#define KLOGF_PREFIX "ps2io: "

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_CMD 0x64

/* Max retry counter when checking the status register if we can read from
 * PS2_DATA or write to PS2_DATA || PS2_CMD */
#define MAX_TRY_COUNT 5

/* Timeout for reading data from a ps/2 device */
TIMEOUT_CREATE(dev_timeout, 50);

static bool g_ps2_controller_dual = false;

static int ps2io_write_byte(u8 data, u8 port)
{
    u8 tries = 0;
    u8 st = port_inb(PS2_STATUS);
    while (st & (1 << 1))
    {
        if (tries++ > MAX_TRY_COUNT)
            return -ETIMEDOUT;

        st = port_inb(PS2_STATUS);
    }

    port_outb(data, port);
    return 0;
}

ERR_OR(u8) ps2io_read_data_byte()
{
    u8 tries = 0;
    u8 st = port_inb(PS2_STATUS);
    while (!(st & (1 << 0)))
    {
        if (tries++ > MAX_TRY_COUNT)
            return ERR(u8, -ETIMEDOUT);

        st = port_inb(PS2_STATUS);
    }

    return VALUE(u8, port_inb(PS2_DATA));
}

u8 ps2io_read_data_byte_nochk()
{
    return port_inb(PS2_DATA);
}

static int ps2io_disable_and_flush_devices()
{
    /* Disable first ps2 device */
    int st = ps2io_write_byte(0xAD, PS2_CMD);
    if (st < 0)
        return st;

    /* Disable second ps2 device */
    st = ps2io_write_byte(0xA7, PS2_CMD);
    if (st < 0)
        return st;

    /* Flush internat controller buffers */
    port_inb(PS2_DATA);
    return 0;
}

static ERR_OR(u8) ps2io_read_config_byte()
{
    int st = ps2io_write_byte(0x20, PS2_CMD);
    if (st < 0)
        return ERR(u8, st);

    return ps2io_read_data_byte();
}

static int ps2io_write_config_byte(u8 config)
{
    int st = ps2io_write_byte(0x60, PS2_CMD);
    if (st < 0)
        return st;

    return ps2io_write_byte(config, PS2_DATA);
}

static int ps2io_perform_self_test()
{
    int st = ps2io_write_byte(0xAA, PS2_CMD);
    if (st < 0)
        return st;

    ERR_OR(u8) res = ps2io_read_data_byte();
    if (res.error)
    {
        return res.error;
    }
    else if (res.value != 0x55)
    {
        KLOGF(ERR, "Controller failed self test with 0x%X", res.value);
        return -EINVAL;
    }

    return 0;
}

static int ps2io_test_port(u8 portn)
{
    if (portn > 1)
        TODO_FATAL();

    int st = ps2io_write_byte(0xAB, PS2_CMD);
    if (st < 0)
        return st;

    ERR_OR(u8) dev_res = ps2io_read_data_byte();
    if (dev_res.error)
    {
        return dev_res.error;
    }
    else if (dev_res.value > 0)
    {
        KLOGF(ERR, "Port 1 test responded with: %X", dev_res.value);
        return -EINVAL;
    }

    return 0;
}

int ps2io_set_interrupts(bool state, u8 portn)
{
    ASSERT(portn == 1 || portn == 2);
    ERR_OR(u8) config_res = ps2io_read_config_byte();
    if (config_res.error)
        return config_res.error;

    if (portn == 1)
    {
        if (state)
            config_res.value |= (1 << 0);
        else
            config_res.value &= ~(1 << 0);
    }
    else
    {
        if (state)
            config_res.value |= (1 << 1);
        else
            config_res.value &= ~(1 << 1);
    }

    return ps2io_write_config_byte(config_res.value);
}

int ps2io_set_scanning(bool state, u8 portn)
{
    if (portn > 1)
        TODO_FATAL();

    int st = ps2io_write_byte(state ? 0xF4 : 0xF5, PS2_DATA);
    if (st < 0)
        return st;

    st = port_inb(PS2_DATA);
    return st == 0xFA; // ack
}

static int ps2io_reset_device(u8 portn)
{
    if (portn > 1)
        TODO_FATAL();

    bool ack = false;
    /* Reset */
    int st = ps2io_write_byte(0xFF, PS2_DATA);
    if (st < 0)
        return st;

    TIMEOUT_START(dev_timeout);
    while (true)
    {
        if (TIMEOUT_DONE(dev_timeout))
            return -ETIMEDOUT;

        ERR_OR(u8) data_res = ps2io_read_data_byte();

        if (!ack && data_res.value == 0xFA)
        {
            ack = true;
        }
        else if (ack)
        {
            if (data_res.value == 0xFC || data_res.value == 0xFD)
            {
                KLOGF(ERR, "Device 1 failed reset self test.");
                return -EINVAL;
            }
            else if (data_res.value == 0xFE)
            {
                TODO(); // send again
            }
            else if (data_res.value == 0xAA)
                break;
        }
    }

    /* We disable scanning, until the device's driver takes over and does
     * whatever */
    return ps2io_set_scanning(false, 1);
}

static int ps2io_identify_device(u8 portn)
{
    if (portn > 1)
        TODO_FATAL();

    bool ack = false;
    u8 ident_data[2] = {0xFF};
    u8 ident_data_cursor = 0;
    u8 prev_data = 0;

    /* Scanning is already disabled */
    /* Send identify cmd */
    ps2io_write_byte(0xF2, PS2_DATA);

    TIMEOUT_START(dev_timeout);
    while (true)
    {
        if (TIMEOUT_DONE(dev_timeout))
            break;

        u8 data = port_inb(PS2_DATA);

        if (data == 0xFA)
        {
            ack = true;
        }
        else if (ack)
        {
            if (ident_data_cursor >= 2)
                break;

            if (data != prev_data)
                ident_data[ident_data_cursor++] = data;

            prev_data = data;
        }
    }

    /* If nothing's been written it's a timeout */
    if (ident_data[0] == 0xFF)
        return -ETIMEDOUT;

    ERR_OR_PTR(SerialIODevice) dev_res;
    if (ident_data[0] == 0xAB)
    {
        // Some type of keyboard
        switch (ident_data[1])
        {
        case 0x83:
        case 0xC1:
            // MF2 keyboard (tf does mf stand for?)
            dev_res = serialio_new_dev("ps2kbd", SERIAL_NOMAJ, 1, NULL);
            break;

        default:
            dev_res.error = -ENODEV;
            break;
        }
    }

    return dev_res.error;
}

static int ps2io_init_controller()
{
    /* FIXME: We just assume the ps2 controller is present, but we should check
     for it's existance. */

    int st = ps2io_disable_and_flush_devices();
    if (st < 0)
        return st;

    /* Read config byte cmd */
    ERR_OR(u8) config_res = ps2io_read_config_byte();
    if (config_res.error)
        return config_res.error;

    g_ps2_controller_dual = (bool)(config_res.value & (1 << 5));

    /* Disable interrupts for port1 */
    config_res.value &= ~(1 << 0);
    /* Disable interrupts for port2 */
    config_res.value &= ~(1 << 1);
    /* Disable translation for port1, port2 doesn't have translation. */
    config_res.value &= ~(1 << 6);

    /* Write the modified config byte back */
    ps2io_write_config_byte(config_res.value);

    /* Perform self test. Presumably this command can reset some ps2
     * controllers, so we reset it's state after this. */
    st = ps2io_perform_self_test();
    if (st < 0)
        return st;

    ps2io_disable_and_flush_devices();
    ps2io_write_config_byte(config_res.value);

    /* Test port1 */
    st = ps2io_test_port(1);
    if (st < 0)
        return st;
    /* TODO: Test port2 */

    /* Enable first device */
    st = ps2io_write_byte(0xAE, PS2_CMD);
    if (st < 0)
        return st;

    /* Disable scanning for first device */
    st = ps2io_set_scanning(false, 1);
    if (st < 0)
        return st;

    /* Reset first device */
    return ps2io_reset_device(1);
}

static int ps2io_main()
{
    int st = ps2io_init_controller();
    if (st < 0)
        return st;

    st = ps2io_identify_device(1);
    if (st < 0)
        KLOGF(ERR, "First device identification failed with: %d", st);

    if (g_ps2_controller_dual)
        TODO();

    return 0;
}

static int ps2io_exit()
{
    return 0;
}

MODULE g_ps2io_module = {
    .name = "ps2io",
    .main = ps2io_main,
    .exit = ps2io_exit,
    .stage = MODULE_STAGE3};
