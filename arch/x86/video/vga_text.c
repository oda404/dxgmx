
#include<dxgmx/video/vga_text.h>

#define VGA_CURSOR_START_REG 0x0A
#define VGA_MISC_PORT_R      0x3CC
#define VGA_MISC_PORT_W      0x3C2
#define VGA_CURS_START_REG   0x0A
#define VGA_IDX_REG_1        0x3C4
#define VGA_IDX_REG_2        0x3CE
#define VGA_IDX_REG_3        0x3D4

static uint16_t *vga_buff_base = (uint16_t *)0xB8000;

static inline void outb(uint8_t value, unsigned short int port)
{
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(unsigned short int port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void vga_init()
{
    /* set bit 0 of misc port */
    uint8_t state = inb(VGA_MISC_PORT_R);
    outb(state | (1 << 0), VGA_MISC_PORT_W);
}

int vga_put_char(char c, uint8_t fg, uint8_t bg, uint8_t row, uint8_t col)
{
    if(row >= VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;
    if(col >= VGA_MAX_WIDTH)
        return VGA_ERR_INVALID_WIDTH;
    
    *(vga_buff_base + VGA_MAX_WIDTH * row + col) |= c << 0 | (uint16_t) fg << 8 | (uint16_t) bg << 12;

    return 0;
}

int vga_clear_char(uint8_t row, uint8_t col)
{
    if(row >= VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;
    if(col >= VGA_MAX_WIDTH)
        return VGA_ERR_INVALID_WIDTH;
    
    *(vga_buff_base  + VGA_MAX_WIDTH * row + col) = 0;

    return 0;
}

int vga_clear_row(uint8_t row)
{
    if(row >= VGA_MAX_HEIGHT)
        return VGA_ERR_INVALID_HEIGHT;
    
    for(size_t i = 0; i < VGA_MAX_WIDTH; ++i)
        vga_clear_char(row, i);

    return 0;
}

void vga_disable_cursor()
{
    outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    uint8_t state = inb(VGA_IDX_REG_3 + 1);
    outb(state | (1 << 5), VGA_IDX_REG_3 + 1);
}

void vga_enable_cursor()
{
    outb(VGA_CURS_START_REG, VGA_IDX_REG_3);
    uint8_t state = inb(VGA_IDX_REG_3 + 1);
    outb(state & ~(1 << 5), VGA_IDX_REG_3 + 1);
}
