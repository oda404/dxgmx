
KERNEL_SRC += \
kernel/bitwise.c \
kernel/kprintf.c \
kernel/kabort.c \
kernel/klog.c \
kernel/kinit_stage2.c \
kernel/kinfo.c \

ifdef CONFIG_STACK_PROT
	KERNEL_SRC += kernel/stack_chk.c 
endif

include $(KERNEL_SRCDIR)/ctype/Makefile
include $(KERNEL_SRCDIR)/errno/Makefile
include $(KERNEL_SRCDIR)/stdlib/Makefile
include $(KERNEL_SRCDIR)/string/Makefile
include $(KERNEL_SRCDIR)/math/Makefile
include $(KERNEL_SRCDIR)/mem/Makefile
include $(KERNEL_SRCDIR)/video/Makefile
include $(KERNEL_SRCDIR)/time/Makefile
