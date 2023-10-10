
include drivers/serialio/module.mk

MODULE_INCLUDEDIRS += \
drivers/x86/ps2io/include

MODULEOBJS += \
drivers/x86/ps2io/ps2io.c.o