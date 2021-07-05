/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/


#ifndef _DXGMX_VIDEO_VGA_TEXT_H
#define _DXGMX_VIDEO_VGA_TEXT_H

#include<stdint.h>
#include<stddef.h>

#define VGA_ERR_INVALID_WIDTH  1
#define VGA_ERR_INVALID_HEIGHT 2
#define VGA_ERR_INVALID_FG     3
#define VGA_ERR_INVALID_BG     4
#define VGA_MAX_WIDTH        80
#define VGA_MAX_HEIGHT       25

enum VGAColor
{
    VGA_COLOR_BLACK   = 0x0,
    VGA_COLOR_BLUE    = 0x1,
    VGA_COLOR_GREEN   = 0x2,
    VGA_COLOR_CYAN    = 0x3,
    VGA_COLOR_RED     = 0x4,
    VGA_COLOR_MEGENTA = 0x5,
    VGA_COLOR_BROWN   = 0x6,
    VGA_COLOR_L_GRAY  = 0x7,
    VGA_COLOR_D_GRAY  = 0x8,
    VGA_COLOR_L_BLUE  = 0x9,
    VGA_COLOR_L_GREEN = 0xA,
    VGA_COLOR_L_CYAN  = 0xB,
    VGA_COLOR_L_RED   = 0xC,
    VGA_COLOR_PINK    = 0xD,
    VGA_COLOR_YELLOW  = 0xE,
    VGA_COLOR_WHITE   = 0xF
};

void vga_init();
void vga_enable_cursor();
void vga_disable_cursor();
int vga_clear_row(uint8_t row);
int vga_put_char(char c, uint8_t fg, uint8_t bg, uint8_t row, uint8_t col);
int vga_clear_char(uint8_t row, uint8_t col);

#endif // _DXGMX_VIDEO_VGA_TEXT_H
