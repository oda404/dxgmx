
include drivers/ramfs/module.mk

EXTRA_DEFINES += -DCONFIG_DEVFS

MODULE_INCLUDEDIRS += \
drivers/devfs/include

MODULEOBJS += \
drivers/devfs/devfs.c.o \