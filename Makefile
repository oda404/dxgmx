# Copyright 2023 Alexandru Olaru.
# Distributed under the MIT license.

VER_MAJ            := 0
VER_MIN            := 19
PATCH_N            := 0
CODENAME           := angel_attack

SRCROOT            := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

CONFIG_FILE        ?= $(SRCROOT)/config.mk

### MISC DIRECTORIES ###
BUILDDIR           := $(SRCROOT)/build
SYSROOT            ?= $(SRCROOT)/sysroot

SYSROOT_DISK_MNTPOINT ?= /mnt/dxgmx-sysroot

### OBJ FILES ###
ARCHOBJS           :=
KERNELOBJS         :=
LDSCRIPT           :=
MODULEOBJS         :=
MODULE_INCLUDEDIRS :=
EXPORT_APIS        :=

-include $(CONFIG_FILE)

### BINARY/ISO PATHS ###
KERNEL_BIN   ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
KERNEL_ISO   ?= $(KERNEL_BIN).iso

.PHONY += all
all: $(KERNEL_BIN)

# Die if we are not using a LLVM toolchain
LLVM=$(shell tools/is-llvm.sh)
ifneq ($(LLVM),1)
    $(error You need an LLVM toolchain to build dxgmx.)
endif

# Translate arch into src arch.
SRCARCH           := $(shell tools/arch.sh --to-srcarch $(CONFIG_ARCH))

TARGET_TRIP       := $(CONFIG_ARCH)-unknown-dxgmx

### BASE FLAGS ###
CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x \
-fno-omit-frame-pointer -ffreestanding \
-fno-builtin -march=$(CONFIG_ARCH) \
-fno-pie -fno-pic --target=$(TARGET_TRIP)

INCLUDEDIRS := -Iinclude 

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

PRETTY_PRINT      := tools/pretty-print.sh

ifeq ($(CONFIG_DEBUG_INFO),y) 
	EXTRA_CFLAGS += -g
endif

ifdef CONFIG_OPTIMIZATIONS
	EXTRA_CFLAGS += -O$(CONFIG_OPTIMIZATIONS)
endif

# Include main subdirs
-include arch/$(SRCARCH)/sub.mk
include kernel/sub.mk
include include/sub.mk
include tools/sub.mk

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

SYSCALL_DEFS       := include/dxgmx/generated/syscall_defs.h
CONFIG_DEFS        := include/dxgmx/generated/kconfig.h

CORE_DEPS          := $(CONFIG_DEFS) $(SYSCALL_DEFS) .WAIT

SYSCALLS_COMMON    := kernel/syscalls_common.json

.PHONY             :=

$(KERNEL_BIN): $(CORE_DEPS) $(ALL_OBJS) $(LDSCRIPT)
	@$(PRETTY_PRINT) LD $@
	@$(LD) -T $(LDSCRIPT) $(ALL_OBJS) $(LDFLAGS) -o $(KERNEL_BIN)
	@tools/bake_symbols.sh $(KERNEL_BIN)

$(SYSCALL_DEFS): $(TOOL_SYSCALLS_GEN) $(SYSCALLS_COMMON)
	@$(PRETTY_PRINT) SYSDEFS $@
	@$(TOOL_SYSCALLS_GEN) --common-defs $(SYSCALLS_COMMON) --output $@

$(CONFIG_DEFS): $(TOOL_KCONFIG_H_GENERATOR) $(CONFIG_FILE)
	@$(PRETTY_PRINT) KCONFIG $@
	@$(TOOL_KCONFIG_H_GENERATOR) -i $(CONFIG_FILE) -o $(CONFIG_DEFS)

-include $(CDEPS)
$(BUILDDIR)/%.c.o: %.c $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%.S.o: %.S $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

-include $(CMODDEPS)
$(BUILDDIR)/%_mod.c.o: %.c $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "CC MOD" $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(SMODDEPS)
$(BUILDDIR)/%_mod.S.o: %.S $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "AS MOD" $<
	@$(AS) -c $< $(CFLAGS) -o $@

.PHONY += install_apis
install_apis:
	@tools/install-apis.sh --sysroot $(SYSROOT) --apis "$(EXPORT_APIS)"

.PHONY += apisinfo
apisinfo:
	@echo The following headers are to be installed under sysroot?/usr/include/dxgmx/:
	@echo $(EXPORT_APIS) | tr " " "\n"

.PHONY += install
install: $(KERNEL_BIN)
	@mkdir -p $(SYSROOT)/boot
	@mkdir -p $(SYSROOT)/usr/include
	@$(MAKE) install_apis
	@cp -u $(KERNEL_BIN) $(SYSROOT)/boot/

.PHONY += clean 
clean:
	@rm -f $(ALL_OBJS)
	@rm -rf $(BUILDDIR)/tools/*
	@rm -f $(CDEPS) $(ASMDEPS) $(CMODDEPS) $(SMODDEPS)
	@rm -rf include/dxgmx/generated/*

.PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

.PHONY += buildinfo
buildinfo:
	@echo CC: $(CC)
	@echo AS: $(AS)
	@echo LD: $(LD)
	@echo Target triplet: $(TARGET_TRIP)
	@echo System root: $(SYSROOT)
	@echo CFLAGS: $(CFLAGS)
	@echo LDFLAGS: $(LDFLAGS)

.PHONY += conf
conf: $(TOOL_NCURSES_CONFIG)
	@$(PRETTY_PRINT) CONFIG $(CONFIG_FILE)
	@$(TOOL_NCURSES_CONFIG) -c $(CONFIG_FILE) -r $(SRCROOT)/root_config_menu.json
