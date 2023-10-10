
include drivers/fb/module.mk

MODULEOBJS += \
drivers/fbsink/fbsink.c.o \
drivers/fbsink/psf.c.o \

MISCOBJS += \
drivers/fbsink/ruscii_8x16.psfu.o

$(BUILDDIR)/%.psfu.o: %.psfu
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) OBJCOPY $<
# 	Absolutely horrible
	@$(OBJCOPY) -O elf32-i386 -B i686 -I binary $< $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers_fbsink_ruscii_8x16_psfu_start=_builtin_psfu_start $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers_fbsink_ruscii_8x16_psfu_end=_builtin_psfu_end $@
