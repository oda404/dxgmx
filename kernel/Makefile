
KERNEL_SRC += \
$(KERNEL_SRCDIR)/kprintf.c \
$(KERNEL_SRCDIR)/kabort.c \
$(KERNEL_SRCDIR)/klog.c \
$(KERNEL_SRCDIR)/kinit_stage2.c \
$(KERNEL_SRCDIR)/ksyms.c \
$(KERNEL_SRCDIR)/kimg.c \
$(KERNEL_SRCDIR)/module.c \
$(KERNEL_SRCDIR)/panic.c \
$(KERNEL_SRCDIR)/userspace.c \

KINIT_STAGE3_SRC := $(KERNEL_SRCDIR)/kinit_stage3.c

include $(KERNEL_SRCDIR)/mem/Makefile
include $(KERNEL_SRCDIR)/utils/Makefile
include $(KERNEL_SRCDIR)/crypto/Makefile
include $(KERNEL_SRCDIR)/kstdlib/Makefile
include $(KERNEL_SRCDIR)/storage/Makefile
include $(KERNEL_SRCDIR)/fs/Makefile
include $(KERNEL_SRCDIR)/elf/Makefile
include $(KERNEL_SRCDIR)/proc/Makefile
include $(KERNEL_SRCDIR)/syscalls/Makefile
