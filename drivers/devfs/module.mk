
include drivers/ramfs/module.mk

EXTRA_DEFINES += -DCONFIG_DEVFS

MODULE_INCLUDEDIRS += \
$(DRIVERS_SRCDIR)/devfs/include

MODULEOBJS += \
$(DRIVERS_SRCDIR)/devfs/devfs.c.o \