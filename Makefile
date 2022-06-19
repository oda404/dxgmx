# Copyright 2021-2022 Alexandru Olaru.
# Distributed under the MIT license.

KERNEL_NAME       := dxgmx

VER_MAJ           := 0
VER_MIN           := 11
PATCH_N           := 3
CODENAME          := angel_attack

ifeq ($(DXGMX_ARCH),)
    $(error DXGMX_ARCH is undefined!)
endif

DXGMX_TOOLCHAIN_ROOT ?= /

### MISC DIRECTORIES ###
BUILDDIR          := build/
SYSROOTDIR        := $(PWD)/sysroot/
SCRIPTSDIR        := scripts/

SRCARCH           := $(shell $(SCRIPTSDIR)/arch.sh --to-srcarch $(DXGMX_ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch: '$(DXGMX_ARCH)')
endif

DXGMX_TARGET_TRIP := $(DXGMX_ARCH)-unknown-dxgmx

### SRC DIRECTORIES ###
ARCH_SRCDIR       := arch/$(SRCARCH)/
INIT_SRCDIR       := init/
KERNEL_SRCDIR     := kernel/
FS_SRCDIR         := fs/
INCLUDE_SRCDIR    := include/

### BASE FLAGS ###
CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x \
--sysroot=$(SYSROOTDIR) -fno-omit-frame-pointer \
-ffreestanding -fno-builtin -I$(INCLUDE_SRCDIR) \
-march=$(DXGMX_ARCH)

LDFLAGS           := -nostdlib -L $(DXGMX_TOOLCHAIN_ROOT)/usr/lib \

MACROS            := \
-D_DXGMX_ -DDXGMX_VER_MAJ=$(VER_MAJ) \
-DDXGMX_VER_MIN=$(VER_MIN) -DDXGMX_PATCH_N=$(PATCH_N) \
-DDXGMX_CODENAME='"$(CODENAME)"' 

WARNINGS          := -Wall -Wextra -Wshadow \
-Werror=implicit-int \
-Werror=incompatible-pointer-types \
-Wunused -Wnull-dereference -Wdouble-promotion \
-Wformat=2 -Wmisleading-indentation #-Wsign-conversion

ifeq ($(LLVM),1)
	CFLAGS += --target=$(DXGMX_TARGET_TRIP)
	LDFLAGS += -L $(DXGMX_TOOLCHAIN_ROOT)/usr/lib/linux # ??
endif

MAKEFLAGS         += --no-print-directory

### BINARY/ISO PATHS ###
KERNEL_BIN_NAME   := $(KERNEL_NAME)-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
KERNEL_BIN_PATH   := $(KERNEL_BIN_NAME)
KERNEL_ISO_PATH   ?= $(KERNEL_BIN_PATH).iso

PRETTY_PRINT  = $(SCRIPTSDIR)/pretty-print.sh
export PRETTY_PRINT

### SRC FILES ###
ARCH_SRC          :=
INIT_SRC          :=
KERNEL_SRC        :=
FS_SRC            :=
LDSCRIPT          :=
HEADERS           :=
MODULES_SRC       :=

ifdef TARGET_FILE
    include $(TARGET_FILE)
else
    $(warning No TARGET_FILE specified!)
endif

# The above variables will be populated recursively by
# these included makefiles.
include $(ARCH_SRCDIR)/Makefile
include $(INIT_SRCDIR)/Makefile
include $(KERNEL_SRCDIR)/Makefile
include $(FS_SRCDIR)/Makefile
include $(INCLUDE_SRCDIR)/Makefile

CFLAGS            += $(EXTRA_CFLAGS) $(WARNINGS) $(EXTRA_WARNINGS) $(MACROS) $(EXTRA_MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS) $(LIBS) $(EXTRA_LIBS)

$(info CC: $(CC))
$(info AS: $(AS))
$(info LD: $(LD))
$(info Build target name: $(shell [ $(shell expr length "$(TARGET_NAME)") -gt 0 ] && echo $(TARGET_NAME) || echo No target ))
$(info Target architecture: $(DXGMX_ARCH))
$(info Target triplet: $(DXGMX_TARGET_TRIP))
$(info Toolchain root: $(DXGMX_TOOLCHAIN_ROOT))
$(info System root: $(SYSROOTDIR))
$(info CFLAGS: $(CFLAGS))
$(info LDFLAGS: $(LDFLAGS))

ALL_SRC := $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC) $(FS_SRC)

# Filter out and add TARGET_NAME to each object.
COBJS             := $(filter %.c, $(ALL_SRC))
COBJS             := $(COBJS:%.c=%_$(BUILDTARGET_NAME).c.o)
COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CDEPS             := $(COBJS:%.o=%.d)

ASMOBJS           := $(filter %.S, $(ALL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(BUILDTARGET_NAME).S.o)
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))
ASMDEPS           := $(ASMOBJS:%.o=%.d)

MODOBJS           := $(filter %.c, $(MODULES_SRC))
MODOBJS           := $(MODOBJS:%.c=%_mod_$(BUILDTARGET_NAME).c.o)
MODOBJS           := $(addprefix $(BUILDDIR)/, $(MODOBJS))
MODDEPS           := $(MODOBJS:%.o=%.d)

DXGMX_DEPS        := \
$(COBJS) $(ASMOBJS) $(LDSCRIPT) $(MODOBJS)

DXGMX_COMMON_DEPS := Makefile $(BUILDCONFIG) $(BUILDTARGET)

PHONY             :=

PHONY += all
all: $(KERNEL_BIN_PATH)

$(KERNEL_BIN_PATH): $(DXGMX_DEPS) $(DXGMX_COMMON_DEPS)
	@$(PRETTY_PRINT) LD $(notdir $(KERNEL_BIN_NAME))
	@$(LD) -T $(LDSCRIPT) $(COBJS) $(ASMOBJS) $(MODOBJS) $(LDFLAGS) -o $(KERNEL_BIN_PATH)

	@[ -f build/image.img ] || $(SCRIPTSDIR)/create-disk.sh -p build/image.img
	@NM=$(NM) OBJCOPY=$(OBJCOPY) $(SCRIPTSDIR)/bake_symbols.sh $(KERNEL_BIN_PATH) 

	@cp $(KERNEL_BIN_PATH) $(SYSROOTDIR)/boot/

-include $(CDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(MODDEPS)
$(BUILDDIR)/%_mod_$(BUILDTARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "CC MOD" $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

PHONY += iso 
iso: $(KERNEL_ISO_PATH)
$(KERNEL_ISO_PATH): $(KERNEL_BIN_PATH)
	$(MAKE)
	@mkdir -p $(SYSROOTDIR)/boot/grub
	@echo "timeout=0"                      >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "menuentry \"$(KERNEL_BIN_NAME)\" {"    >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "	multiboot /boot/$(KERNEL_BIN_NAME)"   >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "}"                              >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(KERNEL_ISO_PATH) $(SYSROOTDIR)

PHONY += iso-run 
iso-run:
	$(MAKE) iso
	DXGMX_DISK=build/image.img DXGMX_MEM=128M \
		$(SCRIPTSDIR)/run.sh -i $(KERNEL_ISO_PATH) -a $(DXGMX_ARCH)

PHONY += run 
run:
	$(MAKE)
	DXGMX_DISK=build/image.img DXGMX_MEM=128M \
		$(SCRIPTSDIR)/run.sh -k $(KERNEL_BIN_PATH) -a $(DXGMX_ARCH)

PHONY += clean 
clean:
	@rm -f $(COBJS) $(ASMOBJS) $(MODOBJS)
	@rm -f $(CDEPS) $(ASMDEPS) $(MODDEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

.PHONY: $(PHONY)
