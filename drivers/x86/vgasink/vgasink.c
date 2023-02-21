/**
 * Copyright 2023 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/kimg.h>
#include <dxgmx/kstdio/kstdio.h>
#include <dxgmx/module.h>
#include <dxgmx/x86/portio.h>

#define VGA_CURSOR_START_REG 0x0A
#define VGA_MISC_PORT_R 0x3CC
#define VGA_MISC_PORT_W 0x3C2
#define VGA_CURS_START_REG 0x0A
#define VGA_IDX_REG_1 0x3C4
#define VGA_IDX_REG_2 0x3CE
#define VGA_IDX_REG_3 0x3D4

typedef enum E_VGAColor
{
    VGA_COLOR_BLACK = 0x0,
    VGA_COLOR_BLUE = 0x1,
    VGA_COLOR_GREEN = 0x2,
    VGA_COLOR_CYAN = 0x3,
    VGA_COLOR_RED = 0x4,
    VGA_COLOR_MEGENTA = 0x5,
    VGA_COLOR_BROWN = 0x6,
    VGA_COLOR_L_GRAY = 0x7,
    VGA_COLOR_D_GRAY = 0x8,
    VGA_COLOR_L_BLUE = 0x9,
    VGA_COLOR_L_GREEN = 0xA,
    VGA_COLOR_L_CYAN = 0xB,
    VGA_COLOR_L_RED = 0xC,
    VGA_COLOR_PINK = 0xD,
    VGA_COLOR_YELLOW = 0xE,
    VGA_COLOR_WHITE = 0xF
} VGAColor;

typedef struct S_VGATextRenderingContext
{
    VGAColor fg;
    VGAColor bg;
    u8 width;
    u8 height;
    u8 current_row;
    u8 current_col;

    u16* buf_vaddr;
} VGATextRenderingContext;

static void vgasink_disable_cursor()
{
    port_outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    u8 state = port_inb(VGA_IDX_REG_3 + 1);
    port_outb(state | (1 << 5), VGA_IDX_REG_3 + 1);
}

static int
vgasink_clear_char_at(u8 row, u8 col, const VGATextRenderingContext* ctx)
{
    if (row >= ctx->height || col >= ctx->width)
        return false;

    ctx->buf_vaddr[ctx->width * row + col] = 0;

    return 0;
}

static int vgasink_clear_row(u8 row, const VGATextRenderingContext* ctx)
{
    if (row >= ctx->height)
        return false;

    for (size_t i = 0; i < ctx->width; ++i)
        vgasink_clear_char_at(row, i, ctx);

    return 0;
}

static void vgasink_clear_screen(const VGATextRenderingContext* ctx)
{
    for (size_t i = 0; i < ctx->height * ctx->width; ++i)
        ctx->buf_vaddr[i] = 0;
}

static int vgasink_init(KOutputSink* sink)
{
    VGATextRenderingContext* ctx = sink->ctx;

    ctx->buf_vaddr = (u16*)(0xB8000 + (ptr)kimg_map_offset());
    u8 state = port_inb(VGA_MISC_PORT_R);
    port_outb(state | (1 << 0), VGA_MISC_PORT_W);
    ctx->height = 25;
    ctx->width = 80;
    ctx->current_col = 0;
    ctx->current_row = 0;
    ctx->fg = VGA_COLOR_WHITE;
    ctx->bg = VGA_COLOR_BLACK;

    vgasink_disable_cursor();
    vgasink_clear_screen(ctx);

    return 0;
}

static void vgasink_scroll(size_t lines, const VGATextRenderingContext* ctx)
{
    while (lines--)
    {
        for (size_t i = 1; i < ctx->height; ++i)
        {
            for (size_t k = 0; k < ctx->width; ++k)
            {
                ctx->buf_vaddr[ctx->width * (i - 1) + k] =
                    ctx->buf_vaddr[ctx->width * i + k];
            }
        }
        vgasink_clear_row(ctx->height - 1, ctx);
    }
}

static int vgasink_newline(KOutputSink* sink)
{
    VGATextRenderingContext* ctx = sink->ctx;

    ctx->current_col = 0;
    if (++ctx->current_row >= ctx->height)
    {
        vgasink_scroll(1, ctx);
        --ctx->current_row;
    }

    return 0;
}

static int vgasink_print_char(char c, KOutputSink* sink)
{
    VGATextRenderingContext* ctx = sink->ctx;

    if (ctx->current_col >= ctx->width || ctx->current_row >= ctx->height)
        return false;

    ctx->buf_vaddr[ctx->width * ctx->current_row + ctx->current_col] =
        c | (u16)ctx->fg << 8 | (u16)ctx->bg << 12;

    if (++ctx->current_col >= ctx->width)
        vgasink_newline(sink);

    return 0;
}

static VGATextRenderingContext g_vga_ctx;

static KOutputSink g_vgasink = {
    .name = "vga",
    .type = KOUTPUT_TERMINAL,
    .output_char = vgasink_print_char,
    .newline = vgasink_newline,
    .ctx = &g_vga_ctx};

static int vgasink_main()
{
    int st = vgasink_init(&g_vgasink);
    if (st < 0)
        return st;

    st = kstdio_register_sink(&g_vgasink);
    if (st < 0)
    {

        // FIXME: destroy vgasink
    }

    return st;
}

static int vgasink_exit()
{
    return 0;
}

MODULE g_vgasink_module = {
    .name = "vgasink",
    .main = vgasink_main,
    .exit = vgasink_exit,
    .stage = MODULE_STAGE1};
