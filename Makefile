# Copyright 2023 Alexandru Olaru.
# Distributed under the MIT license.

KERNEL_NAME       := dxgmx
VER_MAJ           := 0
VER_MIN           := 19
PATCH_N           := 0
CODENAME          := angel_attack

ifeq ($(DXGMX_ARCH),)
    $(error DXGMX_ARCH is undefined!)
endif

SYSROOT_DISK_MNTPOINT  ?= /mnt/dxgmx-sysroot

### MISC DIRECTORIES ###
BUILDDIR          := build
SYSROOT           ?= $(PWD)/sysroot
TOOLS_SRCDIR      := tools

### BINARY/ISO PATHS ###
KERNEL_BIN   := $(KERNEL_NAME)-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
KERNEL_ISO   ?= $(KERNEL_BIN).iso

PHONY += all
all: $(KERNEL_BIN)

# Die if we are not using a LLVM toolchain
LLVM=$(shell $(TOOLS_SRCDIR)/is-llvm.sh)
ifneq ($(LLVM),1)
    $(error You need an LLVM toolchain to build dxgmx.)
endif

# Translate arch into src arch.
SRCARCH           := $(shell $(TOOLS_SRCDIR)/arch.sh --to-srcarch $(DXGMX_ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch: '$(DXGMX_ARCH)')
endif

DXGMX_TARGET_TRIP := $(DXGMX_ARCH)-unknown-dxgmx

### SRC DIRECTORIES ###
ARCH_SRCDIR       := arch/$(SRCARCH)
KERNEL_SRCDIR     := kernel
INCLUDE_SRCDIR    := include
DRIVERS_SRCDIR    := drivers

### BASE FLAGS ###
CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x \
-fno-omit-frame-pointer -ffreestanding \
-fno-builtin -march=$(DXGMX_ARCH) \
-fno-pie -fno-pic --target=$(DXGMX_ARCH)-unknown-dxgmx

INCLUDEDIRS := -I$(INCLUDE_SRCDIR) 

DEFINES            := \
-D_KERNEL \
-DCONFIG_VER_MAJ=$(VER_MAJ) \
-DCONFIG_VER_MIN=$(VER_MIN) -DCONFIG_PATCH_N=$(PATCH_N) \
-DCONFIG_CODENAME='"$(CODENAME)"' 

WARNINGS          := -Wall -Wextra -Wshadow \
-Werror=implicit-int \
-Werror=incompatible-pointer-types \
-Wunused -Wnull-dereference -Wdouble-promotion \
-Wformat=2 -Wmisleading-indentation #-Wsign-conversion

LDFLAGS           := -nostdlib

MAKEFLAGS         += --no-print-directory

PRETTY_PRINT  = $(TOOLS_SRCDIR)/pretty-print.sh
export PRETTY_PRINT

### OBJ FILES ###
ARCHOBJS           :=
KERNELOBJS         :=
LDSCRIPT           :=
MODULEOBJS         :=
MODULE_INCLUDEDIRS :=
EXPORT_APIS        :=

ifdef TARGET_FILE
    include $(TARGET_FILE)
else
    $(warning No TARGET_FILE specified!)
endif

# Include main subdirs
include $(ARCH_SRCDIR)/sub.mk
include $(KERNEL_SRCDIR)/sub.mk
include $(INCLUDE_SRCDIR)/sub.mk
include $(TOOLS_SRCDIR)/sub.mk

# Add module include directories to INCLUDEDIRS
INCLUDEDIRS += $(patsubst %, -I%, $(MODULE_INCLUDEDIRS))

CFLAGS             += $(EXTRA_CFLAGS) $(WARNINGS) $(EXTRA_WARNINGS) $(INCLUDEDIRS) $(DEFINES) $(EXTRA_DEFINES)
LDFLAGS            += $(EXTRA_LDFLAGS)

CORE_OBJS          := $(ARCHOBJS) $(KERNELOBJS)

# Filter out each object.
COBJS              := $(filter %.c.o, $(CORE_OBJS))
COBJS              := $(addprefix $(BUILDDIR)/, $(COBJS))
CDEPS              := $(COBJS:%.o=%.d)

ASMOBJS            := $(filter %.S.o, $(CORE_OBJS))
ASMOBJS            := $(addprefix $(BUILDDIR)/, $(ASMOBJS))
ASMDEPS            := $(ASMOBJS:%.o=%.d)

CMODOBJS           := $(filter %.c.o, $(MODULEOBJS))
CMODOBJS           := $(CMODOBJS:%.c.o=%_mod.c.o)
CMODOBJS           := $(addprefix $(BUILDDIR)/, $(CMODOBJS))
CMODDEPS           := $(CMODOBJS:%.o=%.d)

SMODOBJS           := $(filter %.S.o, $(MODULEOBJS))
SMODOBJS           := $(SMODOBJS:%.S.o=%_mod.S.o)
SMODOBJS           := $(addprefix $(BUILDDIR)/, $(SMODOBJS))
SMODDEPS           := $(SMODOBJS:%.o=%.d)

MISCOBJS           := $(addprefix $(BUILDDIR)/, $(MISCOBJS))

CMODOBJS           := $(sort $(CMODOBJS))
SMODOBJS           := $(sort $(SMODOBJS))
MISCOBJS           := $(sort $(MISCOBJS))

ALL_OBJS           := $(COBJS) $(ASMOBJS) $(CMODOBJS) $(SMODOBJS) $(MISCOBJS)

DXGMX_COMMON_DEPS  := Makefile $(TARGET_FILE)

SYSCALL_DEFS       := $(INCLUDE_SRCDIR)/dxgmx/syscall_defs.h

PHONY              :=

$(KERNEL_BIN): $(SYSCALL_DEFS) $(ALL_OBJS) $(LDSCRIPT) $(DXGMX_COMMON_DEPS)
	@$(PRETTY_PRINT) LD $@
	@$(LD) -T $(LDSCRIPT) $(ALL_OBJS) $(LDFLAGS) -o $(KERNEL_BIN)

	@[ -f build/image.img ] || $(TOOLS_SRCDIR)/create-disk.sh -p build/image.img
	@$(TOOLS_SRCDIR)/bake_symbols.sh $(KERNEL_BIN)

$(SYSCALL_DEFS): $(TOOL_SYSCALLS_GEN) $(KERNEL_SRCDIR)/syscalls_common.defs
	@$(PRETTY_PRINT) SYSDEFS $@
	@$(TOOL_SYSCALLS_GEN) --common-defs $(KERNEL_SRCDIR)/syscalls_common.defs --output $@

-include $(CDEPS)
$(BUILDDIR)/%.c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%.S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

-include $(CMODDEPS)
$(BUILDDIR)/%_mod.c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "CC MOD" $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(SMODDEPS)
$(BUILDDIR)/%_mod.S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "AS MOD" $<
	@$(AS) -c $< $(CFLAGS) -o $@

PHONY += install_apis
install_apis:
	@$(TOOLS_SRCDIR)/install-apis.sh --sysroot $(SYSROOT) --apis "$(EXPORT_APIS)"

PHONY += install
install: $(KERNEL_BIN)
	@mkdir -p $(SYSROOT)/boot
	@mkdir -p $(SYSROOT)/usr/include
	@$(MAKE) install_apis
	@cp $(KERNEL_BIN) $(SYSROOT)/boot/

PHONY += iso 
iso: $(KERNEL_ISO)
$(KERNEL_ISO): $(KERNEL_BIN)
	$(TOOLS_SRCDIR)/create-iso.sh \
	--sysroot $(SYSROOT) \
	--kernel $(KERNEL_BIN) \
	--out $(KERNEL_ISO)

PHONY += iso-run 
iso-run: $(KERNEL_ISO)
	$(TOOLS_SRCDIR)/run-qemu.sh -i $(KERNEL_ISO) -a $(DXGMX_ARCH)

PHONY += run 
run: $(KERNEL_BIN)
	$(TOOLS_SRCDIR)/run-qemu.sh -k $(KERNEL_BIN) -a $(DXGMX_ARCH)

PHONY += clean 
clean:
	@rm -f $(COBJS) $(ASMOBJS) $(CMODOBJS) $(MISCOBJS)
	@rm -rf $(BUILDDIR)/$(TOOLS_SRCDIR)/*
	@rm -f $(CDEPS) $(ASMDEPS) $(MODDEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $(SYSCALL_DEFS)
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

PHONY += mount-root
mount-root: build/image.img
	sudo $(TOOLS_SRCDIR)/mount-root.sh \
	--image-path build/image.img \
	--mountpoint $(SYSROOT_DISK_MNTPOINT) \
	--cachefile $(BUILDDIR)/root-loopdev

PHONY += unmount-root
unmount-root:
	sudo $(TOOLS_SRCDIR)/unmount-root.sh --cachefile $(BUILDDIR)/root-loopdev

PHONY += syncroot
syncroot:
	sudo $(TOOLS_SRCDIR)/syncroot.sh \
	--sysroot $(SYSROOT) \
	--image-path build/image.img \
	--mountpoint $(SYSROOT_DISK_MNTPOINT) \
	--cachefile $(BUILDDIR)/root-loopdev

PHONY += buildinfo
buildinfo:
	@echo CC: $(CC)
	@echo AS: $(AS)
	@echo LD: $(LD)
	@echo Build target name: $(shell [ $(shell expr length "$(TARGET_NAME)") -gt 0 ] && echo $(TARGET_NAME) || echo No target )
	@echo Target architecture: $(DXGMX_ARCH)
	@echo Target triplet: $(DXGMX_TARGET_TRIP)
	@echo System root: $(SYSROOT)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

PHONY += conf
conf: $(TOOL_NCURSES_CONFIG)
	@$(PRETTY_PRINT) CONFIG config
	@$(TOOL_NCURSES_CONFIG) -c config.mk -r $(PWD)/root_config_menu.json

.PHONY: $(PHONY)
