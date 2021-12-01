
KERNEL_SRC += \
$(KERNEL_SRCDIR)/kprintf.c \
$(KERNEL_SRCDIR)/kabort.c \
$(KERNEL_SRCDIR)/klog.c \
$(KERNEL_SRCDIR)/kinit_stage2.c \
$(KERNEL_SRCDIR)/ksyms.c \

ifdef CONFIG_STACK_PROT
	ifneq ($(CONFIG_STACK_PROT),none)
		KERNEL_SRC += $(KERNEL_SRCDIR)/stack_chk.c
	endif 
endif

include $(KERNEL_SRCDIR)/mem/Makefile
include $(KERNEL_SRCDIR)/video/Makefile
include $(KERNEL_SRCDIR)/utils/Makefile
include $(KERNEL_SRCDIR)/crypto/Makefile
include $(KERNEL_SRCDIR)/kstdlib/Makefile
