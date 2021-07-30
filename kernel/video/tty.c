/*
    Copyright Alexandru Olaru.
    Distributed under the MIT license.
*/

#include<dxgmx/video/tty.h>
#include<dxgmx/video/vga_text.h>
#include<dxgmx/string.h>
#include<stdint.h>

static uint16_t g_current_row;
static uint16_t g_current_col;

void tty_init()
{
    vga_init(80, 25);
    vga_disable_cursor();
    tty_clear();
}

void tty_clear()
{
    size_t i;
    for(i = 0; i < vga_get_max_height(); ++i)
    {
        vga_clear_row(i);
    }
    g_current_col = 0;
    g_current_row = 0;
}

size_t tty_print(const char *str, size_t n)
{
    size_t len = strlen(str);
    if(n > len)
        n = len;

    for(size_t i = 0; i < n; ++i)
    {
        switch(str[i])
        {
        case '\n':
            g_current_col = 0;
            ++g_current_row;
            break;
        
        default:
            vga_put_char(
                str[i], 
                VGA_COLOR_WHITE, 
                VGA_COLOR_BLACK,
                g_current_row,
                g_current_col
            );
            ++g_current_col;
            break;
        }

        if(g_current_col >= vga_get_max_width())
        {
            g_current_col = 0;
            ++g_current_row;
        }

        if(g_current_row >= vga_get_max_height())
        {
            vga_scroll(1);
            --g_current_row;
            g_current_col = 0;
        }
    }

    return n;
}
