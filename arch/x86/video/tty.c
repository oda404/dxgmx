
#include<dxgmx/video/tty.h>
#include<dxgmx/video/vga_text.h>
#include<string.h>
#include<stdint.h>

static uint16_t current_row;
static uint16_t current_col;

void tty_init()
{
    tty_clear();
    current_col = 0;
    current_row = 0;
}

void tty_clear()
{
    size_t i;
    for(i = 0; i < VGA_MAX_HEIGHT; ++i)
    {
        vga_clear_row(i);
    }
}

int tty_print(const char *str, size_t n)
{
    size_t len = strlen(str);
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
