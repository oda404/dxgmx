
KERNELOBJS += \
$(KERNEL_SRCDIR)/kabort.c.o \
$(KERNEL_SRCDIR)/klog.c.o \
$(KERNEL_SRCDIR)/kinit_stage2.c.o \
$(KERNEL_SRCDIR)/ksyms.c.o \
$(KERNEL_SRCDIR)/kimg.c.o \
$(KERNEL_SRCDIR)/module.c.o \
$(KERNEL_SRCDIR)/panic.c.o \
$(KERNEL_SRCDIR)/user.c.o \
$(KERNEL_SRCDIR)/kstdio.c.o \
$(KERNEL_SRCDIR)/kboot.c.o \

include $(KERNEL_SRCDIR)/mem/sub.mk
include $(KERNEL_SRCDIR)/utils/sub.mk
include $(KERNEL_SRCDIR)/crypto/sub.mk
include $(KERNEL_SRCDIR)/kstdlib/sub.mk
include $(KERNEL_SRCDIR)/storage/sub.mk
include $(KERNEL_SRCDIR)/fs/sub.mk
include $(KERNEL_SRCDIR)/elf/sub.mk
include $(KERNEL_SRCDIR)/proc/sub.mk
include $(KERNEL_SRCDIR)/syscalls/sub.mk
include $(KERNEL_SRCDIR)/sched/sub.mk
include $(KERNEL_SRCDIR)/time/sub.mk
