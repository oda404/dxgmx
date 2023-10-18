
EXTRA_CFLAGS += -fstack-protector-$(CONFIG_STACK_PROT)

MODULEOBJS += drivers/security/stackprot/stack_chk.c.o
