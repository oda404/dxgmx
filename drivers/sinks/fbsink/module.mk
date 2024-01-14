
MODULEOBJS += \
drivers/sinks/fbsink/fbsink.c.o \
drivers/sinks/fbsink/psf.c.o \

MISCOBJS += \
drivers/sinks/fbsink/ruscii_8x16.psfu.o

ifeq ($(CONFIG_ARCH),x86_64)
	__PSFU_OBJCOPY_O = elf64-x86-64
	__PSFU_OBJCOPY_B = x86-64
else ifeq ($(CONFIG_ARCH),i686)
# I FUCKING LOVE THIS PROGRAM WHYC OULDNT THETY AMKE ALL I*86 VARIANTS WORK???? NOW I HAVE TO DO THISS SHIT
	__PSFU_OBJCOPY_O = elf32-i386
	__PSFU_OBJCOPY_B = i686
endif

$(BUILDDIR)/%.psfu.o: %.psfu $(CONFIG_FILE)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) OBJCOPY $<
# 	Absolutely horrible
	@$(OBJCOPY) -O $(__PSFU_OBJCOPY_O) -B $(__PSFU_OBJCOPY_B) -I binary $< $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers_sinks_fbsink_ruscii_8x16_psfu_start=_builtin_psfu_start $@
	@$(OBJCOPY) --redefine-sym=_binary_drivers_sinks_fbsink_ruscii_8x16_psfu_end=_builtin_psfu_end $@
