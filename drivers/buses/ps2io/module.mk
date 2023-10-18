
MODULE_INCLUDEDIRS += \
drivers/buses/ps2io/include

MODULEOBJS += \
drivers/buses/ps2io/ps2io.c.o

ifdef CONFIG_X86
	MODULEOBJS += drivers/buses/ps2io/ps2io_x86.c.o
endif
