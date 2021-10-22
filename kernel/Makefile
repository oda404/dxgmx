
KERNEL_SRC += \
$(KERNEL_SRCDIR)/bitwise.c \
$(KERNEL_SRCDIR)/kprintf.c \
$(KERNEL_SRCDIR)/kabort.c \
$(KERNEL_SRCDIR)/klog.c \
$(KERNEL_SRCDIR)/kinit_stage2.c \
$(KERNEL_SRCDIR)/kinfo.c \

ifdef CONFIG_STACK_PROT
	ifneq ($(CONFIG_STACK_PROT),none)
		KERNEL_SRC += $(KERNEL_SRCDIR)/stack_chk.c
	endif 
endif

include $(KERNEL_SRCDIR)/ctype/Makefile
include $(KERNEL_SRCDIR)/errno/Makefile
include $(KERNEL_SRCDIR)/stdlib/Makefile
include $(KERNEL_SRCDIR)/string/Makefile
include $(KERNEL_SRCDIR)/math/Makefile
include $(KERNEL_SRCDIR)/mem/Makefile
include $(KERNEL_SRCDIR)/video/Makefile
include $(KERNEL_SRCDIR)/time/Makefile
include $(KERNEL_SRCDIR)/utils/Makefile
include $(KERNEL_SRCDIR)/stdio/Makefile
include $(KERNEL_SRCDIR)/crypto/Makefile
