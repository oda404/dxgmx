
MODULEOBJS += \
drivers/video/fb/fb.c.o 

ifdef CONFIG_DEVFS
	MODULEOBJS += drivers/video/fb/fb_devfs.c.o
endif

MODULE_INCLUDEDIRS += \
drivers/video/fb/include

EXPORT_APIS += \
drivers/video/fb/include/dxgmx/user@fb.h