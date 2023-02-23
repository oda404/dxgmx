# Copyright 2023 Alexandru Olaru.
# Distributed under the MIT license.

KERNEL_NAME       := dxgmx

VER_MAJ           := 0
VER_MIN           := 18
PATCH_N           := 0
CODENAME          := angel_attack

ifeq ($(DXGMX_ARCH),)
    $(error DXGMX_ARCH is undefined!)
endif

SYSROOT_DISK_MNTPOINT  ?= /mnt/dxgmx-sysroot

### MISC DIRECTORIES ###
BUILDDIR          := build/
DXGMX_SYSROOT     ?= $(PWD)/sysroot/
SCRIPTSDIR        := scripts/

SRCARCH           := $(shell $(SCRIPTSDIR)/arch.sh --to-srcarch $(DXGMX_ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch: '$(DXGMX_ARCH)')
endif

DXGMX_TARGET_TRIP := $(DXGMX_ARCH)-unknown-dxgmx

### SRC DIRECTORIES ###
ARCH_SRCDIR       := arch/$(SRCARCH)/
KERNEL_SRCDIR     := kernel/
INCLUDE_SRCDIR    := include/
DRIVERS_SRCDIR    := drivers/

### BASE FLAGS ###
BASE_CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x \
-fno-omit-frame-pointer -ffreestanding \
-fno-builtin -march=$(DXGMX_ARCH) \
-fno-pie -fno-pic --target=$(DXGMX_ARCH)-unknown-dxgmx

INCLUDEDIRS := -I$(INCLUDE_SRCDIR) 

LLVM=$(shell $(SCRIPTSDIR)/is-llvm.sh)

ifneq ($(LLVM),1)
    $(error You need an LLVM toolchain to build dxgmx.)
endif

LDFLAGS           := -nostdlib

MACROS            := \
-DDXGMX_VER_MAJ=$(VER_MAJ) \
-DDXGMX_VER_MIN=$(VER_MIN) -DDXGMX_PATCH_N=$(PATCH_N) \
-DDXGMX_CODENAME='"$(CODENAME)"' 

WARNINGS          := -Wall -Wextra -Wshadow \
-Werror=implicit-int \
-Werror=incompatible-pointer-types \
-Wunused -Wnull-dereference -Wdouble-promotion \
-Wformat=2 -Wmisleading-indentation #-Wsign-conversion

MAKEFLAGS         += --no-print-directory

### BINARY/ISO PATHS ###
KERNEL_BIN_NAME   := $(KERNEL_NAME)-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
KERNEL_BIN_PATH   := $(KERNEL_BIN_NAME)
KERNEL_ISO_PATH   ?= $(KERNEL_BIN_PATH).iso

PRETTY_PRINT  = $(SCRIPTSDIR)/pretty-print.sh
export PRETTY_PRINT

### SRC FILES ###
ARCH_SRC          :=
KERNEL_SRC        :=
LDSCRIPT          :=
HEADERS           :=
MODULES_SRC       :=
KINIT_STAGE3_SRC  :=

ifdef TARGET_FILE
    include $(TARGET_FILE)
else
    $(warning No TARGET_FILE specified!)
endif

# The above variables will be populated recursively by
# these included makefiles.
include $(ARCH_SRCDIR)/Makefile
include $(KERNEL_SRCDIR)/Makefile

BASE_CFLAGS       += $(INCLUDEDIRS) $(WARNINGS) $(MACROS)
CFLAGS            += $(BASE_CFLAGS) $(EXTRA_CFLAGS) $(EXTRA_WARNINGS) $(EXTRA_MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS)

ifeq ($(MAKECMDGOALS),)
    $(info CC: $(CC))
    $(info AS: $(AS))
    $(info LD: $(LD))
    $(info Build target name: $(shell [ $(shell expr length "$(TARGET_NAME)") -gt 0 ] && echo $(TARGET_NAME) || echo No target ))
    $(info Target architecture: $(DXGMX_ARCH))
    $(info Target triplet: $(DXGMX_TARGET_TRIP))
    $(info System root: $(DXGMX_SYSROOT))
    $(info CFLAGS: $(CFLAGS))
    $(info LDFLAGS: $(LDFLAGS))
    $(info )
endif

ALL_SRC := $(ARCH_SRC) $(KERNEL_SRC)

# Filter out and add TARGET_NAME to each object.
COBJS             := $(filter %.c, $(ALL_SRC))
COBJS             := $(COBJS:%.c=%_$(TARGET_NAME).c.o)
COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CDEPS             := $(COBJS:%.o=%.d)

ASMOBJS           := $(filter %.S, $(ALL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(TARGET_NAME).S.o)
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))
ASMDEPS           := $(ASMOBJS:%.o=%.d)

MODOBJS           := $(filter %.c, $(MODULES_SRC))
MODOBJS           := $(MODOBJS:%.c=%_mod_$(TARGET_NAME).c.o)
MODOBJS           := $(addprefix $(BUILDDIR)/, $(MODOBJS))
MODDEPS           := $(MODOBJS:%.o=%.d)

KINIT_STAGE3_OBJ := $(filter %.c, $(KINIT_STAGE3_SRC))
KINIT_STAGE3_OBJ := $(KINIT_STAGE3_OBJ:%.c=%_kinit3_$(TARGET_NAME).c.o)
KINIT_STAGE3_OBJ := $(addprefix $(BUILDDIR)/, $(KINIT_STAGE3_OBJ))
KINIT_STAGE3_DEP := $(KINIT_STAGE3_OBJ:%.o=%.d)

DXGMX_DEPS        := \
$(COBJS) $(KINIT_STAGE3_OBJ) $(ASMOBJS) $(LDSCRIPT) $(MODOBJS)

DXGMX_COMMON_DEPS := Makefile $(BUILDCONFIG) $(BUILDTARGET)

PHONY             :=

PHONY += all
all: $(KERNEL_BIN_PATH)

$(KERNEL_BIN_PATH): $(DXGMX_DEPS) $(DXGMX_COMMON_DEPS)
	@$(PRETTY_PRINT) LD $(notdir $(KERNEL_BIN_NAME))
	@$(LD) -T $(LDSCRIPT) $(COBJS) $(KINIT_STAGE3_OBJ) $(ASMOBJS) $(MODOBJS) $(LDFLAGS) -o $(KERNEL_BIN_PATH)

	@[ -f build/image.img ] || $(SCRIPTSDIR)/create-disk.sh -p build/image.img
	@$(SCRIPTSDIR)/bake_symbols.sh $(KERNEL_BIN_PATH)

-include $(CDEPS)
$(BUILDDIR)/%_$(TARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(KINIT_STAGE3_DEP)
$(BUILDDIR)/%_kinit3_$(TARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(BASE_CFLAGS) -o $@

-include $(MODDEPS)
$(BUILDDIR)/%_mod_$(TARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "CC MOD" $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(TARGET_NAME).S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

PHONY += iso 
iso: $(KERNEL_ISO_PATH)
$(KERNEL_ISO_PATH): $(KERNEL_BIN_PATH)
	$(MAKE)
	$(SCRIPTSDIR)/create-iso.sh \
	--sysroot $(DXGMX_SYSROOT) \
	--kernel $(KERNEL_BIN_NAME) \
	--out $(KERNEL_ISO_PATH)

PHONY += iso-run 
iso-run:
	$(MAKE) iso
	$(SCRIPTSDIR)/run-qemu.sh -i $(KERNEL_ISO_PATH) -a $(DXGMX_ARCH)

PHONY += run 
run:
	$(MAKE)
	$(SCRIPTSDIR)/run-qemu.sh -k $(KERNEL_BIN_PATH) -a $(DXGMX_ARCH)

PHONY += clean 
clean:
	@rm -f $(COBJS) $(ASMOBJS) $(MODOBJS) $(KINIT_STAGE3_OBJ)
	@rm -f $(CDEPS) $(ASMDEPS) $(MODDEPS) $(KINIT_STAGE3_DEP)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

PHONY += mount-root
mount-root: build/image.img
	sudo $(SCRIPTSDIR)/mount-root.sh \
	--image-path build/image.img \
	--mountpoint $(SYSROOT_DISK_MNTPOINT) \
	--cachefile $(BUILDDIR)/root-loopdev

PHONY += unmount-root
unmount-root:
	sudo $(SCRIPTSDIR)/unmount-root.sh --cachefile $(BUILDDIR)/root-loopdev

PHONY += syncroot
syncroot:
	sudo $(SCRIPTSDIR)/syncroot.sh \
	--sysroot $(DXGMX_SYSROOT) \
	--image-path build/image.img \
	--mountpoint $(SYSROOT_DISK_MNTPOINT) \
	--cachefile $(BUILDDIR)/root-loopdev

.PHONY: $(PHONY)
