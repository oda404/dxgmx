
include drivers/serialio/module.mk

MODULE_INCLUDEDIRS += \
$(DRIVERS_SRCDIR)/x86/ps2io/include

MODULEOBJS += \
$(DRIVERS_SRCDIR)/x86/ps2io/ps2io.c.o