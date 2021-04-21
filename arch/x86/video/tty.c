
#include<dxgmx/video/tty.h>
#include<dxgmx/video/vga_text.h>
#include<dxgmx/string.h>
#include<stdint.h>

static uint16_t current_row;
static uint16_t current_col;

void tty_init()
{
    vga_init();
    vga_disable_cursor();
    tty_clear();
}

void tty_clear()
{
    size_t i;
    for(i = 0; i < VGA_MAX_HEIGHT; ++i)
    {
        vga_clear_row(i);
    }
    current_col = 0;
    current_row = 0;
}

int tty_print(const char *str, size_t n)
{
    size_t len = __strlen(str);
    if(n > len)
        return -1;
    size_t i;
    for(i = 0; i < n; ++i)
    {
        if(current_col >= VGA_MAX_WIDTH)
        {
            current_col = 0;
            ++current_row;
        }
        // TODO handle scrolling

        switch(str[i])
        {
        case '\n':
            current_col = 0;
            ++current_row;
            break;
        
        default:
            vga_put_char(
                str[i], 
                VGA_COLOR_WHITE, 
                VGA_COLOR_BLACK,
                current_row,
                current_col
            );
            ++current_col;
            break;
        }
    }

    return n;
}
