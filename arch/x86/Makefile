
ARCH_SRC += \
$(ARCH_SRCDIR)/kinit_stage1.c \
$(ARCH_SRCDIR)/cpu.c \
$(ARCH_SRCDIR)/cpuid.c \
$(ARCH_SRCDIR)/cmos.c \
$(ARCH_SRCDIR)/rtc.c \
$(ARCH_SRCDIR)/acpi.c \
$(ARCH_SRCDIR)/abandon_ship.c \
$(ARCH_SRCDIR)/clocksource.c \
$(ARCH_SRCDIR)/gdt.c \

LDSCRIPT := \
$(ARCH_SRCDIR)/linker.ld

include $(ARCH_SRCDIR)/boot/Makefile
include $(ARCH_SRCDIR)/video/Makefile
include $(ARCH_SRCDIR)/int/Makefile
include $(ARCH_SRCDIR)/mem/Makefile
