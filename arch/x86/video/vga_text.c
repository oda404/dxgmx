
#include<dxgmx/video/vga_text.h>

#define VGA_CURSOR_START_REG 0x0A
#define VGA_PORT             0x3D5

static uint16_t *vga_buff_base = (uint16_t *)0xB8000;

static size_t no_std_lib_having_ass_strlen(const char *str)
{
    size_t len = 0;
    while(str[len++] != '\0');
    return len;
}

int vga_put_char(char c, uint8_t fg, uint8_t bg, uint8_t row, uint8_t col)
{
    if(row > VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;
    if(col > VGA_MAX_WIDTH)
        return VGA_ERR_INVALID_WIDTH;
    
    *(vga_buff_base + VGA_MAX_WIDTH * row + col) |= c << 0 | (uint16_t) fg << 8 | (uint16_t) bg << 12;

    return 0;
}

int vga_clear_char(uint8_t row, uint8_t col)
{
    if(row > VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;
    if(col > VGA_MAX_WIDTH)
        return VGA_ERR_INVALID_WIDTH;
    
    *(vga_buff_base  + VGA_MAX_WIDTH * row + col) = 0;

    return 0;
}

int vga_clear_row(uint8_t row)
{
    if(row > VGA_MAX_HEIGHT - 1)
        return VGA_ERR_INVALID_HEIGHT;
    
    for(size_t i = 0; i < VGA_MAX_WIDTH; ++i)
        vga_clear_char(row, i);

    return 0;
}

int vga_put_str(const char *str, uint8_t fg, uint8_t bg, uint8_t row)
{
    if(row > VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;

    vga_clear_row(row);
    for(size_t i = 0; i < no_std_lib_having_ass_strlen(str); ++i)
    {
        vga_put_char(str[i], fg, bg, row, i);
    }

    return 0;
}

static inline void outb(unsigned char value, unsigned short int port)
{
    asm volatile ("outb %0, %1"::"a"(value), "Nd"(port));
}

void vga_disable_cursor()
{
    outb(VGA_CURSOR_START_REG | 1 << 5, VGA_PORT);
}

void vga_enable_cursor()
{
    outb(VGA_CURSOR_START_REG & 1 << 5, VGA_PORT);
}
