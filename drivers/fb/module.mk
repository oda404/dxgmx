
MODULEOBJS += \
drivers/fb/fb.c.o \
drivers/fb/fb_ioctl.c.o \

MODULE_INCLUDEDIRS += \
drivers/fb/include

EXPORT_APIS += \
drivers/fb/include/dxgmx/fb_user.h:fb_defs.h