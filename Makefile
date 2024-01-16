# Copyright 2023 Alexandru Olaru.
# Distributed under the MIT license.

VER_MAJ            := 0
VER_MIN            := 19
PATCH_N            := 0
CODENAME           := angel_attack

SRCROOT            := $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

CONFIG_FILE        ?= $(SRCROOT)/config.mk

### MISC DIRECTORIES ###
BUILDDIR           ?= $(SRCROOT)/build
TOOLSDIR           := $(SRCROOT)/tools
SYSROOT            ?= /

KERNEL_BIN         ?= $(SRCROOT)/dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)

CC                 ?= cc
AS                 ?= as
LD                 ?= ld
NM                 ?= nm
OBJCOPY            ?= objcopy

.PHONY += all
all: $(KERNEL_BIN)

### OBJ FILES ###
ARCHOBJS           :=
KERNELOBJS         :=
LDSCRIPT           :=
MODULEOBJS         :=
MODULE_INCLUDEDIRS :=
EXPORT_APIS        :=

DRY_DEPS           := install_apis apisinfo buildinfo conf clean mrclean help
ifeq ($(filter $(DRY_DEPS),$(MAKECMDGOALS)),)
	CHEW := 1
else
	CHEW := 0
endif

ifeq ($(CHEW), 1)
	LLVM=$(shell $(TOOLSDIR)/is-llvm.sh)
	ifneq ($(LLVM),1)
        $(error You need an LLVM toolchain to build dxgmx. Current CC is $(shell which ${CC}))
	endif
endif

-include $(CONFIG_FILE)

# Translate arch into src arch.
SRCARCH           := $(shell $(TOOLSDIR)/arch.sh --to-srcarch $(CONFIG_ARCH))
# Some programs want x86-64 instead of x86_64, this is what this does
NORMARCH          := $(shell $(TOOLSDIR)/arch.sh --to-normarch $(CONFIG_ARCH))

TARGET_TRIP       := $(CONFIG_ARCH)-unknown-dxgmx

### BASE FLAGS ###
CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x \
-fno-omit-frame-pointer -ffreestanding \
-fno-builtin -march=$(NORMARCH) \
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

PRETTY_PRINT      := $(TOOLSDIR)/pretty-print.sh

ifeq ($(CONFIG_DEBUG_INFO),y) 
	EXTRA_CFLAGS += -g
endif

ifdef CONFIG_OPTIMIZATIONS
	EXTRA_CFLAGS += -O$(CONFIG_OPTIMIZATIONS)
endif

# Include main subdirs
-include $(SRCROOT)arch/$(SRCARCH)/sub.mk
include $(SRCROOT)kernel/sub.mk
include $(SRCROOT)include/sub.mk
include $(TOOLSDIR)/sub.mk

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

SYSCALL_DEFS       := $(SRCROOT)/include/dxgmx/generated/syscall_defs.h
CONFIG_DEFS        := $(SRCROOT)/include/dxgmx/generated/kconfig.h

CORE_DEPS          := $(CONFIG_DEFS) $(SYSCALL_DEFS) .WAIT

SYSCALLS_COMMON    := $(SRCROOT)/kernel/syscalls_common.json

$(KERNEL_BIN): $(CORE_DEPS) $(ALL_OBJS) $(LDSCRIPT)
	@$(PRETTY_PRINT) LD $(shell basename $@)
	@$(LD) -T $(LDSCRIPT) $(ALL_OBJS) $(LDFLAGS) -o $(KERNEL_BIN)
	@$(TOOLSDIR)/bake_symbols.sh $(KERNEL_BIN)

$(SYSCALL_DEFS): $(TOOL_SYSCALLS_GEN) $(SYSCALLS_COMMON)
	@$(PRETTY_PRINT) SYSDEFS $(patsubst $(SRCROOT)/%,%,$@)
	@$(TOOL_SYSCALLS_GEN) --common-defs $(SYSCALLS_COMMON) --output $@

$(CONFIG_DEFS): $(TOOL_KCONFIG_H_GENERATOR) $(CONFIG_FILE)
	@$(PRETTY_PRINT) KCONFIG $(patsubst $(SRCROOT)/%,%,$@)
	@$(TOOL_KCONFIG_H_GENERATOR) -i $(CONFIG_FILE) -o $(CONFIG_DEFS)

-include $(CDEPS)
$(BUILDDIR)/%.c.o: %.c $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $(patsubst $(BUILDDIR)/%,%,$@)
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%.S.o: %.S $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $(patsubst $(BUILDDIR)/%,%,$@)
	@$(AS) -c $< $(CFLAGS) -o $@

-include $(CMODDEPS)
$(BUILDDIR)/%_mod.c.o: %.c $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "CC MOD" $(patsubst $(BUILDDIR)/%,%,$@)
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(SMODDEPS)
$(BUILDDIR)/%_mod.S.o: %.S $(CORE_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "AS MOD" $(patsubst $(BUILDDIR)/%,%,$@)
	@$(AS) -c $< $(CFLAGS) -o $@

.PHONY += install
install: $(KERNEL_BIN)
	@if [ "$(shell realpath $(SYSROOT))" == "/" ]; then\
		echo "You're trying to install the kernel on /. If this is intentional uncomment this check in the root Makefile.";\
		exit 1;\
	fi

	@mkdir -p $(SYSROOT)/boot
	@mkdir -p $(SYSROOT)/usr/include
	@$(MAKE) install_apis
	@cp -u $(KERNEL_BIN) $(SYSROOT)/boot/

.PHONY += install_apis
install_apis:
	@if [ "$(shell realpath $(SYSROOT))" == "/" ]; then\
		echo "You're trying to install the kernel headers on /. If this is intentional uncomment this check in the root Makefile.";\
		exit 1;\
	fi

	@$(TOOLSDIR)/install-apis.sh --sysroot $(SYSROOT) --apis "$(EXPORT_APIS)"

.PHONY += apisinfo
apisinfo:
	@echo $(EXPORT_APIS) | tr " " "\n"

.PHONY += clean 
clean:
	@$(PRETTY_PRINT) CLEAN $(shell basename ${BUILDDIR})
	@rm -f $(shell find ${BUILDDIR} -type f -name '*.o')
	@rm -f $(shell find ${BUILDDIR} -type f -name '*.d')

	@$(PRETTY_PRINT) CLEAN generated
	@rm -f $(SRCROOT)/include/dxgmx/generated/*

.PHONY += mrclean 
mrclean:
	@$(MAKE) clean

	@$(PRETTY_PRINT) RM "all linked kernels"
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]$$')

	@$(PRETTY_PRINT) RM build
	@rm -r $(BUILDDIR)/* 2> /dev/null || true

.PHONY += buildinfo
buildinfo:
	@echo "Version        : $(VER_MAJ).$(VER_MIN).$(PATCH_N) - $(CODENAME)"
	@echo "Target triplet : $(TARGET_TRIP)"
	@echo "CONFIG         : $(shell readlink -f $(CONFIG_FILE))"
	@echo "SYSROOT        : $(SYSROOT)"
	@echo "BUILDDIR       : $(BUILDDIR)"
	@echo "CC             : $(shell which $(CC))"
	@echo "AS             : $(shell which $(AS))"
	@echo "LD             : $(shell which $(LD))"
	@echo "NM             : $(shell which $(NM))"
	@echo "OBJCOPY        : $(shell which $(OBJCOPY))"

.PHONY += conf
conf: $(TOOL_NCURSES_CONFIG)
	@$(PRETTY_PRINT) CONFIG $(CONFIG_FILE)
	@$(TOOL_NCURSES_CONFIG) -c $(CONFIG_FILE) -r $(SRCROOT)/root_config_menu.json

.PHONY += help
help:
	@echo "dxgmx kernel"
	@echo ""
	@echo "Required environment:"
	@echo "  CC           - path to C compiler"
	@echo "  AS           - path to assembler"
	@echo "  LD           - path to linker"
	@echo "  NM           - path to nm"
	@echo "  OBJCOPY      - path to objcopy"
	@echo "  HOSTCC       - path to C compiler for compiling host binaries"
	@echo "  HOSTCXX      - path to C++ compiler for compiling host binaries"
	@echo "  SYSROOT      - path to sysroot where the kernel is going to install itself and it's userspace headers"
	@echo ""
	@echo "Note: dxgmx only supports building with llvm/clang. CC,AS,LD,NM,OBJCOPY all need to be llvm (tested 15.0.0). The host compilers can be anything."
	@echo ""
	@echo "Optional environment:"
	@echo "  CONFIG_FILE  - path to config file used to build the kernel. Default is config.mk"
	@echo "  BUILDDIR     - path to the build directory. Default is build"
	@echo "  KERNEL_BIN   - path where to output the kernel binary. Default is dxgmx-<vermaj>.<vermin>.<patchn>"
	@echo ""
	@echo "Targets:"
	@echo "  install      - install the kernel and it's userspace headers"
	@echo "  install_apis - install the kernel userspace headers"
	@echo "  apisinfo     - print which kernel userspace headers are to be installed"
	@echo "  clean        - clean every compiled object file + dependency file + generated file"
	@echo "  mrclean      - clean + all kernel images + everything else in the build directory"
	@echo "  buildinfo    - print information about the build"
	@echo "  conf         - configure the kernel using ncurses"
	@echo "  help         - print this wall of text and die"
	@echo ""


