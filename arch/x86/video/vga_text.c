/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#include <dxgmx/x86/portio.h>
#include <dxgmx/x86/vga_text.h>

#define VGA_CURSOR_START_REG 0x0A
#define VGA_MISC_PORT_R 0x3CC
#define VGA_MISC_PORT_W 0x3C2
#define VGA_CURS_START_REG 0x0A
#define VGA_IDX_REG_1 0x3C4
#define VGA_IDX_REG_2 0x3CE
#define VGA_IDX_REG_3 0x3D4

extern u8 _kernel_map_offset[];
static u16* g_vga_buf_base = (u16*)(0xB8000 + (ptr)_kernel_map_offset);

void vgatext_init(VGATextRenderingContext* ctx)
{
    /* set bit 0 of misc port */
    u8 state = port_inb(VGA_MISC_PORT_R);
    port_outb(state | (1 << 0), VGA_MISC_PORT_W);
    ctx->height = 25;
    ctx->width = 80;
    ctx->current_col = 0;
    ctx->current_row = 0;
    ctx->fg = VGA_COLOR_WHITE;
    ctx->bg = VGA_COLOR_BLACK;
}

bool vgatext_print_char_at(
    char c, u8 row, u8 col, const VGATextRenderingContext* ctx)
{
    if (row >= ctx->height || col >= ctx->width)
        return false;

    g_vga_buf_base[ctx->width * row + col] =
        c | (u16)ctx->fg << 8 | (u16)ctx->bg << 12;

    return true;
}

bool vgatext_print_char(char c, VGATextRenderingContext* ctx)
{
    if (ctx->current_col >= ctx->width || ctx->current_row >= ctx->height)
        return false;

    g_vga_buf_base[ctx->width * ctx->current_row + ctx->current_col] =
        c | (u16)ctx->fg << 8 | (u16)ctx->bg << 12;

    if (++ctx->current_col >= ctx->width)
        vgatext_newline(ctx);

    return true;
}

bool vgatext_clear_char_at(u8 row, u8 col, const VGATextRenderingContext* ctx)
{
    if (row >= ctx->height || col >= ctx->width)
        return false;

    g_vga_buf_base[ctx->width * row + col] = 0;

    return true;
}

bool vgatext_clear_row(u8 row, const VGATextRenderingContext* ctx)
{
    if (row >= ctx->height)
        return false;

    for (size_t i = 0; i < ctx->width; ++i)
        vgatext_clear_char_at(row, i, ctx);

    return true;
}

void vgatext_clear_screen(const VGATextRenderingContext* ctx)
{
    for (size_t i = 0; i < ctx->height * ctx->width; ++i)
        g_vga_buf_base[i] = 0;
}

void vgatext_scroll(size_t lines, const VGATextRenderingContext* ctx)
{
    while (lines--)
    {
        for (size_t i = 1; i < ctx->height; ++i)
        {
            for (size_t k = 0; k < ctx->width; ++k)
            {
                g_vga_buf_base[ctx->width * (i - 1) + k] =
                    g_vga_buf_base[ctx->width * i + k];
            }
        }
        vgatext_clear_row(ctx->height - 1, ctx);
    }
}

void vgatext_newline(VGATextRenderingContext* ctx)
{
    ctx->current_col = 0;
    if (++ctx->current_row >= ctx->height)
    {
        vgatext_scroll(1, ctx);
        --ctx->current_row;
    }
}

void vgatext_disable_cursor()
{
    port_outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    u8 state = port_inb(VGA_IDX_REG_3 + 1);
    port_outb(state | (1 << 5), VGA_IDX_REG_3 + 1);
}

void vgatext_enable_cursor()
{
    port_outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    u8 state = port_inb(VGA_IDX_REG_3 + 1);
    port_outb(state & ~(1 << 5), VGA_IDX_REG_3 + 1);
}
