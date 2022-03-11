/**
 * Copyright 2022 Alexandru Olaru.
 * Distributed under the MIT license.
 */

#ifndef _DXGMX_VIDEO_VGA_TEXT_H
#define _DXGMX_VIDEO_VGA_TEXT_H

#include <dxgmx/types.h>

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
} VGATextRenderingContext;

void vgatext_init(VGATextRenderingContext* ctx);
bool vgatext_print_char_at(
    char c, u8 row, u8 col, const VGATextRenderingContext* ctx);
bool vgatext_print_char(char c, VGATextRenderingContext* ctx);
bool vgatext_clear_char_at(u8 row, u8 col, const VGATextRenderingContext* ctx);
bool vgatext_clear_row(u8 row, const VGATextRenderingContext* ctx);
void vgatext_clear_screen(const VGATextRenderingContext* ctx);
void vgatext_scroll(size_t lines, const VGATextRenderingContext* ctx);
void vgatext_newline(VGATextRenderingContext* ctx);
void vgatext_enable_cursor();
void vgatext_disable_cursor();

#endif // _DXGMX_VIDEO_VGA_TEXT_H
