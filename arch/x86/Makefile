
ARCH_SRC += \
arch/x86/init_stage1.c \
arch/x86/init_stage2.c \
arch/x86/cpu.c \
arch/x86/cpuid.c \
arch/x86/cmos.c \
arch/x86/rtc.c \
arch/x86/acpi.c \

LDSCRIPT := \
arch/x86/linker.ld

include $(ARCH_SRCDIR)/boot/Makefile
include $(ARCH_SRCDIR)/video/Makefile
include $(ARCH_SRCDIR)/int/Makefile
include $(ARCH_SRCDIR)/mem/Makefile
include $(ARCH_SRCDIR)/gdt/Makefile
