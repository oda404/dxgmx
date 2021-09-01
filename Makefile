# Copyright 2021 Alexandru Olaru.
# Distributed under the MIT license.


APPNAME           := dxgmx

VER_MAJ           := 0
VER_MIN           := 8
PATCH_N           := 5
CODENAME          := angel_attack

# The makefile expects to find a makefile-like file named 'buildconfig'
# that sets config/toolchain varaibles. If none is found the makefile 
# will fallback on defaults that will not work especially when cross compiling.
# buildconfig example:
#
# HAS_BUILDCONFIG := 1
# IS_CROSS_COMP   := 1
# TARGET_TRIPLET  := x86-dxgmx-elf
# CC              := <CC path>
# BUILD_TARGET    := debug
-include buildconfig

HAS_BUILDCONFIG   ?= 0
ifeq ($(MAKECMDGOALS),)
    ifeq ($(HAS_BUILDCONFIG), 0)
        $(warning Building without a buildconfig file...)
    endif
endif

### DIRECTORIES ###
INCLUDEDIR        := include
BUILDDIR          := build
SYSROOTDIR        := $(BUILDDIR)/sysroot
SCRIPTSDIR        := scripts

TARGET_TRIPLET    ?= x86-dxgmx-elf
IS_CROSS_COMP     ?= 1

BUILD_TARGET      ?= debug
include $(BUILDDIR)/targets/$(BUILD_TARGET).dbt

ARCH              := $(shell $(SCRIPTSDIR)/target-triplet-to-arch.sh $(TARGET_TRIPLET))
ifeq ($(ARCH), undefined)
    $(error Couldn't get arch from target triplet)
endif

SRCARCH           := $(shell $(SCRIPTSDIR)/arch.sh --to-srcarch $(ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch $(ARCH))
endif

### SRC/INCLUDE DIRECTORIES ###
ARCH_SRCDIR       := arch/$(SRCARCH)
INIT_SRCDIR       := init
KERNEL_SRCDIR     := kernel

CC                ?= gcc
CXX               ?= g++
LD                ?= g++
AS                ?= gcc

### FLAGS ###
CFLAGS            :=                                      \
-march=i686 -MD -MP -m32 -isystem=/usr/include            \
--sysroot=$(SYSROOTDIR) -fno-omit-frame-pointer           \
-ffreestanding -fno-builtin $(DBT_CFLAGS)                 \

CXXFLAGS          := $(CFLAGS) $(DBT_CXXFLAGS)            \

LDFLAGS           := -nostdlib -lgcc $(DBT_LDFLAGS)       \

MACROS            :=                                      \
-D__dxgmx__ -D_DXGMX_ -D_DXGMX_VER_MAJ_=$(VER_MAJ)        \
-D_DXGMX_VER_MIN_=$(VER_MIN) -D_DXGMX_PATCH_N_=$(PATCH_N) \
-D_DXGMX_CODENAME_='"$(CODENAME)"' $(DBT_MACROS)          \

WARNINGS          := -Wall -Wextra                        \
-Werror-implicit-function-declaration $(DBT_WARNINGS)     \

ifeq ($(SRCARCH), x86)
	MACROS += -D_X86_
endif

MAKEFLAGS         += --no-print-directory

CFLAGS            += $(EXTRA_CFLAGS) $(WARNINGS) $(MACROS)
CXXFLAGS          += $(EXTRA_CXXFLAGS) $(WARNINGS) $(MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS)

# At this point CFLAGS, CXXFLAGS and LDFLAGS should be in their final forms.

### BINARY/ISO PATHS ###
BIN_NAME          ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
BIN_PATH          ?= $(BIN_NAME)
ISO_PATH          ?= $(BIN_PATH).iso

OUTPUT_FORMATTED  = $(SCRIPTSDIR)/output-formatted.sh
export OUTPUT_FORMATTED

### SRC FILES ###
ARCH_SRC          :=
INIT_SRC          :=
KERNEL_SRC        :=
LDSCRIPT          :=

# The above variables will be populated recursively by
# these included makefiles.
include $(ARCH_SRCDIR)/Makefile
include $(INIT_SRCDIR)/Makefile
include $(KERNEL_SRCDIR)/Makefile

COBJS             := $(filter %.c, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
COBJS             := $(COBJS:%.c=%_$(BUILD_TARGET).c.o)

CXXOBJS           := $(filter %.cpp, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
CXXOBJS           := $(CXXOBJS:%.cpp=%_$(BUILD_TARGET).cpp.o)

ASMOBJS           := $(filter %.S, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(BUILD_TARGET).S.o)

COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CXXOBJS           := $(addprefix $(BUILDDIR)/, $(CXXOBJS))
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))

CDEPS             := $(COBJS:%.o=%.d)
CXXDEPS           := $(CXXOBJS:%.o=%.d)
ASMDEPS           := $(ASMOBJS:%.o=%.d)

SYSROOT_DIRS      := \
$(SYSROOTDIR) $(SYSROOTDIR)/boot \
$(SYSROOTDIR)/usr/include/dxgmx

SYSROOT_HEADERS   := $(shell find include -name "*.h" -type f)
SYSROOT_HEADERS   := $(addprefix $(SYSROOTDIR)/usr/, $(SYSROOT_HEADERS))

DXGMX_DEPS        := $(SYSROOT_DIRS) $(SYSROOT_HEADERS) \
$(COBJS) $(CXXOBJS) $(ASMOBJS) $(LD_SCRIPT) Makefile

PHONY             :=

PHONY += all
all: $(BIN_PATH)

$(BIN_PATH): $(DXGMX_DEPS)
	@$(OUTPUT_FORMATTED) LD $(notdir $(BIN_NAME))
	@$(LD) -T $(LDSCRIPT) $(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDFLAGS) -Wl,-Map,$(BUILDDIR)/$(BIN_NAME).map -o $(BIN_PATH)

	@cp $(BIN_PATH) $(SYSROOTDIR)/boot/


-include $(CDEPS)
$(BUILDDIR)/%_$(BUILD_TARGET).c.o: %.c Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(CXXDEPS)
$(BUILDDIR)/%_$(BUILD_TARGET).cpp.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CXX $<
	@$(CXX) -c $< $(CXXFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(BUILD_TARGET).S.o: %.S Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

$(SYSROOT_DIRS):
	@mkdir -p $(SYSROOT_DIRS)

$(SYSROOTDIR)/usr/%.h: %.h
	@mkdir -p $(dir $@) 2> /dev/null || true
	@cp -ru $< $@

PHONY += kernel_headers 
# kernel headers 
kernel_headers: 
	@cp -ru $(INCLUDEDIR)/* $(SYSROOTDIR)/usr/include

PHONY += iso 
iso: $(ISO_PATH)
$(ISO_PATH): $(BIN_PATH)
	$(MAKE)
	@mkdir -p $(SYSROOTDIR)/boot/grub
	@echo "timeout=0"                      >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "menuentry \"$(BIN_NAME)\" {"    >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "	multiboot /boot/$(BIN_NAME)"   >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@echo "}"                              >> $(SYSROOTDIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO_PATH) $(SYSROOTDIR)

PHONY += iso-run 
iso-run:
	$(MAKE) iso
	qemu-system-x86_64 --enable-kvm -m 2G -cpu host -cdrom $(ISO_PATH)

PHONY += run 
run:
	$(MAKE)
	$(SCRIPTSDIR)/run.sh \
	--kernel-path $(BIN_PATH) \
	--arch $(ARCH)

PHONY += clean 
clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -rf $(BUILDDIR)/arch $(BUILDDIR)/init $(BUILDDIR)/kernel \
	$(BUILDDIR)/sysroot

.PHONY: $(PHONY)
