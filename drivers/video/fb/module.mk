
MODULEOBJS += \
drivers/video/fb/fb.c.o \
drivers/video/fb/fb_ioctl.c.o \

MODULE_INCLUDEDIRS += \
drivers/video/fb/include

EXPORT_APIS += \
drivers/video/fb/include/dxgmx/fb_user.h:fb_defs.h