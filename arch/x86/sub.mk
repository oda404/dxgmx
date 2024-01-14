
ARCHOBJS += \
arch/x86/kinit_arch.c.o \
arch/x86/cpu.c.o \
arch/x86/cpuid.c.o \
arch/x86/cmos.c.o \
arch/x86/gdt.c.o \
arch/x86/serial.c.o \
arch/x86/panic.c.o \
arch/x86/syscalls.c.o \
arch/x86/task.c.o \

ifeq ($(CONFIG_ARCH),x86_64)
    EXTRA_CFLAGS += -m64 -mno-red-zone
else ifeq ($(CONFIG_ARCH),i686)
    EXTRA_CFLAGS += -m32
endif

LDSCRIPT := \
arch/x86/linker.ld

INCLUDEDIRS += -Iarch/x86/include

include arch/x86/boot/sub.mk
include arch/x86/int/sub.mk
include arch/x86/mem/sub.mk
include arch/x86/user/sub.mk
