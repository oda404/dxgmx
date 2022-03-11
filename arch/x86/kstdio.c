/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/attrs.h>
#include <dxgmx/kstdio.h>
#include <dxgmx/todo.h>
#include <dxgmx/x86/serial.h>
#include <dxgmx/x86/vga_text.h>

static VGATextRenderingContext g_vgatext_ctx;
static bool g_serial_debug = false;

static _INIT void kstdio_print_serial_debug(const char* buf, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        serial_write(buf[i], 0x3F8);
}

_INIT bool kstdio_init()
{
    vgatext_init(&g_vgatext_ctx);
    vgatext_disable_cursor();
    vgatext_clear_screen(&g_vgatext_ctx);

    return true;
}

size_t kstdio_write(const char* buf, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        switch (buf[i])
        {
        case '\n':
            vgatext_newline(&g_vgatext_ctx);
            break;

        case '\t':
            for (size_t k = 0; k < 4; ++k)
                vgatext_print_char(' ', &g_vgatext_ctx);
            break;

        case '\r':
            g_vgatext_ctx.current_col = 0;
            break;

        default:
            vgatext_print_char(buf[i], &g_vgatext_ctx);
            break;
        }
    }

    if (g_serial_debug)
        kstdio_print_serial_debug(buf, n);

    return n;
}

_INIT void kstdio_set_serial_debug(bool enabled)
{
    g_serial_debug = enabled;

    SerialPort serial_port = {
        .port = 0x3F8,
        .baudrate = SERIAL_BAUD_9600,
        .databits = SERIAL_DATA_BITS_8,
        .stopbits = SERIAL_STOP_BITS_1,
        .parity = SERIAL_PARITY_NONE};

    if (!serial_config_port(&serial_port))
    {
        g_serial_debug = false;
        klogln(WARN, "kstdio: Failed to enable serial debugging.");
    }
}
