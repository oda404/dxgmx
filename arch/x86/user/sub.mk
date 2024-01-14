
ifeq ($(CONFIG_ARCH),x86_64)
    ARCHOBJS += arch/x86/user/user64.S.o
else ifeq ($(CONFIG_ARCH),i686)
    ARCHOBJS += arch/x86/user/user32.S.o
endif
