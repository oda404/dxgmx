/**
 * Copyright 2021 Alexandru Olaru.
 * Distributed under the MIT license.
*/

#include<dxgmx/x86/vga_text.h>
#include<dxgmx/x86/portio.h>

#define VGA_CURSOR_START_REG 0x0A
#define VGA_MISC_PORT_R      0x3CC
#define VGA_MISC_PORT_W      0x3C2
#define VGA_CURS_START_REG   0x0A
#define VGA_IDX_REG_1        0x3C4
#define VGA_IDX_REG_2        0x3CE
#define VGA_IDX_REG_3        0x3D4

static u16 *g_vga_buf_base = (u16 *)0xC00B8000;
static u8  g_vga_max_w     = 80;
static u8  g_vga_max_h     = 0;

void vga_init(u8 w, u8 h)
{
    /* set bit 0 of misc port */
    u8 state = port_inb(VGA_MISC_PORT_R);
    port_outb(state | (1 << 0), VGA_MISC_PORT_W);
    g_vga_max_w = w;
    g_vga_max_h = h;
}

int vga_put_char(char c, u8 fg, u8 bg, u8 row, u8 col)
{
    if(row >= g_vga_max_h)
        return VGA_ERR_INVALID_HEIGHT;
    if(col >= g_vga_max_w)
        return VGA_ERR_INVALID_WIDTH;
    
    *(g_vga_buf_base + g_vga_max_w * row + col) = 
        c << 0 | (u16) fg << 8 | (u16) bg << 12;

    return 0;
}

int vga_clear_char(u8 row, u8 col)
{
    if(row >= g_vga_max_h)
        return VGA_ERR_INVALID_HEIGHT;
    if(col >= g_vga_max_w)
        return VGA_ERR_INVALID_WIDTH;
    
    *(g_vga_buf_base + g_vga_max_w * row + col) = 0;

    return 0;
}

int vga_clear_row(u8 row)
{
    if(row >= g_vga_max_h)
        return VGA_ERR_INVALID_HEIGHT;
    
    for(size_t i = 0; i < g_vga_max_w; ++i)
        vga_clear_char(row, i);

    return 0;
}

void vga_disable_cursor()
{
    port_outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    u8 state = port_inb(VGA_IDX_REG_3 + 1);
    port_outb(state | (1 << 5), VGA_IDX_REG_3 + 1);
}

void vga_enable_cursor()
{
    port_outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    u8 state = port_inb(VGA_IDX_REG_3 + 1);
    port_outb(state & ~(1 << 5), VGA_IDX_REG_3 + 1);
}

u8 vga_get_max_width()
{
    return g_vga_max_w;
}

u8 vga_get_max_height()
{
    return g_vga_max_h;
}

void vga_scroll(size_t lines)
{
    while(lines--)
    {
        for(size_t i = 1; i < g_vga_max_h; ++i)
        {
            for(size_t k = 0; k < g_vga_max_w; ++k)
            {
                *(g_vga_buf_base + g_vga_max_w * (i - 1) + k) = 
                *(g_vga_buf_base + g_vga_max_w * i + k); 
            }
        }
        vga_clear_row(g_vga_max_h - 1);
    }
}
