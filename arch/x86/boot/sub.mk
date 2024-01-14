

ifeq ($(CONFIG_ARCH),x86_64)
	ARCHOBJS += arch/x86/boot/entry64.S.o
else ifeq ($(CONFIG_ARCH),i686)
	ARCHOBJS += arch/x86/boot/entry32.S.o
# Because before we get to long mode we still have to go through protected mode
# this piece of code still needs to be compiled as 32bit
# $(BUILDDIR)/arch/x86/boot/entry32.S.o: arch/x86/boot/entry32.S $(CORE_DEPS)
# 	@mkdir -p $(dir $@)
# 	@$(PRETTY_PRINT) AS $<
# Let's hope that puttin -m32 here works for all compilers to overwrite
# the previously specified -m64. Works for mine at least
# 	@$(AS) -c $< $(CFLAGS) -m32 -o $@
# 	@$(OBJCOPY) -O elf64-x86-64 $@ $@
endif
