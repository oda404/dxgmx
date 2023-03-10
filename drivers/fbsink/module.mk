
MODULES_SRC += \
$(DRIVERS_SRCDIR)/fbsink/fbsink.c \
$(DRIVERS_SRCDIR)/fbsink/psf.c \

MISCOBJS += \
$(DRIVERS_SRCDIR)/fbsink/ruscii_8x16.psfu.o

$(BUILDDIR)/%.psfu.o: %.psfu
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) OBJCOPY $<
# 	Absolutely horrible
	@$(OBJCOPY) -O elf32-i386 -B i686 -I binary $< $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers__fbsink_ruscii_8x16_psfu_start=_builtin_psfu_start $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers__fbsink_ruscii_8x16_psfu_end=_builtin_psfu_end $@
