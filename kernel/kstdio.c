/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

/* FIXME: this file should be architecture agnostic, but it includes
 * architecture specifc stuff. We should separate this stuff into drivers. */

#include <dxgmx/attrs.h>
#include <dxgmx/errno.h>
#include <dxgmx/kmalloc.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/kstdio/sink.h>
#include <dxgmx/todo.h>
#include <dxgmx/video/fb.h>
#include <dxgmx/x86/serial.h>
#include <dxgmx/x86/vga_text.h>

static VGATextRenderingContext g_vgatext_ctx;
static bool g_serial_debug = false;

extern KOutputSink g_fb_koutput_sink;
static bool g_fb_up = false;

static void kstdio_print_serial_debug(const char* buf, size_t n)
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

int kstdio_init_fb()
{
    int st = g_fb_koutput_sink.init(&g_fb_koutput_sink);
    if (st < 0)
        return st;

    g_fb_up = true;

    return 0;
}

size_t kstdio_write(const char* buf, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        switch (buf[i])
        {
        case '\n':
            vgatext_newline(&g_vgatext_ctx);
            if (g_fb_up)
                g_fb_koutput_sink.newline(&g_fb_koutput_sink);
            break;

        case '\t':
            for (size_t k = 0; k < 4; ++k)
            {
                vgatext_print_char(' ', &g_vgatext_ctx);
                if (g_fb_up)
                    g_fb_koutput_sink.output_char(' ', &g_fb_koutput_sink);
            }
            break;

        case '\r':
            g_vgatext_ctx.current_col = 0;
            break;

        default:
            vgatext_print_char(buf[i], &g_vgatext_ctx);
            if (g_fb_up)
                g_fb_koutput_sink.output_char(buf[i], &g_fb_koutput_sink);
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
