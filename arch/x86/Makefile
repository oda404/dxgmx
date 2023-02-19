
ARCH_SRC += \
$(ARCH_SRCDIR)/kinit_stage1.c \
$(ARCH_SRCDIR)/cpu.c \
$(ARCH_SRCDIR)/cpuid.c \
$(ARCH_SRCDIR)/cmos.c \
$(ARCH_SRCDIR)/acpi.c \
$(ARCH_SRCDIR)/gdt.c \
$(ARCH_SRCDIR)/pci.c \
$(ARCH_SRCDIR)/serial.c \
$(ARCH_SRCDIR)/panic.c \
$(ARCH_SRCDIR)/syscalls.c \
$(ARCH_SRCDIR)/procm.c \
$(ARCH_SRCDIR)/user.S \

MACROS += -D_X86_
ifeq ($(ARCH),x86_64)
    MACROS += -D_X86_64_
else
    CFLAGS += -m32
endif

LDSCRIPT := \
$(ARCH_SRCDIR)/linker.ld

INCLUDEDIRS += -I$(ARCH_SRCDIR)/include

include $(ARCH_SRCDIR)/boot/Makefile
include $(ARCH_SRCDIR)/video/Makefile
include $(ARCH_SRCDIR)/int/Makefile
include $(ARCH_SRCDIR)/mem/Makefile
include $(ARCH_SRCDIR)/time/Makefile
include $(ARCH_SRCDIR)/ata/Makefile
