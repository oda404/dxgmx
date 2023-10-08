
ARCHOBJS += \
$(ARCH_SRCDIR)/kinit_arch.c.o \
$(ARCH_SRCDIR)/cpu.c.o \
$(ARCH_SRCDIR)/cpuid.c.o \
$(ARCH_SRCDIR)/cmos.c.o \
$(ARCH_SRCDIR)/gdt.c.o \
$(ARCH_SRCDIR)/serial.c.o \
$(ARCH_SRCDIR)/panic.c.o \
$(ARCH_SRCDIR)/syscalls.c.o \
$(ARCH_SRCDIR)/proc.c.o \
$(ARCH_SRCDIR)/user.S.o \

ifeq ($(CONFIG_ARCH),i686)
    EXTRA_CFLAGS += -m32
endif

LDSCRIPT := \
$(ARCH_SRCDIR)/linker.ld

INCLUDEDIRS += -I$(ARCH_SRCDIR)/include

include $(ARCH_SRCDIR)/boot/sub.mk
include $(ARCH_SRCDIR)/int/sub.mk
include $(ARCH_SRCDIR)/mem/sub.mk
