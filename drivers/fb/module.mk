
MODULEOBJS += \
$(DRIVERS_SRCDIR)/fb/fb.c.o \
$(DRIVERS_SRCDIR)/fb/fb_ioctl.c.o \

MODULE_INCLUDEDIRS += \
$(DRIVERS_SRCDIR)/fb/include

EXPORT_APIS += \
$(DRIVERS_SRCDIR)/fb/include/dxgmx/fb_user.h:fb_defs.h