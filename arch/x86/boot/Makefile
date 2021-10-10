
ARCH_SRC += \
$(ARCH_SRCDIR)/boot/entry.S \
$(ARCH_SRCDIR)/boot/kstack.c \

ifeq ($(CONFIG_BOOTSPEC),multiboot)
	ARCH_SRC += $(ARCH_SRCDIR)/boot/multiboot.c 
endif
