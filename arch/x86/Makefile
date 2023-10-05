
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

EXTRA_DEFINES += -DCONFIG_X86 -DCONFIG_LITTLE_ENDIAN
ifeq ($(ARCH),x86_64)
    EXTRA_DEFINES += -DCONFIG_X86_64
else
    EXTRA_CFLAGS += -m32
endif

LDSCRIPT := \
$(ARCH_SRCDIR)/linker.ld

INCLUDEDIRS += -I$(ARCH_SRCDIR)/include

include $(ARCH_SRCDIR)/boot/Makefile
include $(ARCH_SRCDIR)/int/Makefile
include $(ARCH_SRCDIR)/mem/Makefile
