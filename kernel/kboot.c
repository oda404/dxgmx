
#include <dxgmx/kboot.h>

ptr _kboot_framebuffer_paddr;
size_t _kboot_framebuffer_width;
size_t _kboot_framebuffer_height;
size_t _kboot_framebuffer_bpp;

bool _kboot_has_initrd;
ptr _kboot_initrd_paddr;
size_t _kboot_initrd_size;
