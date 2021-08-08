# Copyright 2021 Alexandru Olaru.
# Distributed under the MIT license.


APPNAME           := dxgmx

VER_MAJ           := 0
VER_MIN           := 8
PATCH_N           := 2
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
BUILD_TARGET      ?= debug
IS_CROSS_COMP     ?= 1

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
ARCH_INCLUDEDIR   := $(ARCH_DIR)/include
INIT_SRCDIR       := init
KERNEL_SRCDIR     := kernel

CC                ?= gcc
CXX               ?= g++
LD                ?= ld
AR                ?= ar
AS                ?= as

### FLAGS ###
CFLAGS            := -MD -MP -m32 -ffreestanding \
-isystem=/usr/include --sysroot=$(SYSROOTDIR) \

CXXFLAGS          := $(CFLAGS) \

LDFLAGS           := -nostdlib -lgcc

EXTRA_CFLAGS      ?= 
EXTRA_LDFLAGS     ?= 
EXTRA_CXXFLAGS    ?=

DEBUG_CFLAGS      := -g -D_DXGMX_DEBUG_
RELEASE_CFLAGS    := -O2 -D_DXGMX_RELEASE_
DEBUG_CXXFLAGS    := $(DEBUG_CFLAGS)
RELEASE_CXXFLAGS  := $(RELEASE_CFLAGS)

MACROS            := \
-D__dxgmx__ -D_DXGMX_ -D_DXGMX_VER_MAJ_=$(VER_MAJ) \
-D_DXGMX_VER_MIN_=$(VER_MIN) -D_DXGMX_PATCH_N_=$(PATCH_N) \
-D_DXGMX_CODENAME_='"$(CODENAME)"' 

ifeq ($(SRCARCH), x86)
	MACROS += -D_X86_
endif

WARNINGS          := -Wall -Wextra \
-Werror-implicit-function-declaration

MAKEFLAGS         += --no-print-directory

CFLAGS            += $(EXTRA_CFLAGS) $(WARNINGS) $(MACROS)
CXXFLAGS          += $(EXTRA_CXXFLAGS) $(WARNINGS) $(MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS)

# Set release or debug specific CFLAGS.
ifeq ($(BUILD_TARGET), debug)
    CFLAGS += $(DEBUG_CFLAGS)
	CXXFLAGS += $(DEBUG_CXXFLAGS)
else ifeq ($(BUILD_TARGET), release)
    CFLAGS += $(RELEASE_CFLAGS)
	CXXFLAGS += $(RELEASE_CFLAGS)
else
    $(error Unknown BUILD_TARGET=$(BUILD_TARGET))
endif

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
	@$(CXX) -T $(LDSCRIPT) $(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDFLAGS) -o $(BIN_PATH)

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
	@$(CC) -c $< $(CFLAGS) -o $@

$(SYSROOT_DIRS):
	@mkdir -p $(SYSROOT_DIRS)

$(SYSROOTDIR)/usr/%.h: %.h
	@mkdir $(dir $@) 2> /dev/null || true
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
	qemu-system-x86_64 -cdrom $(ISO_PATH)

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
	@rm -rf $(BUILDDIR) # yikes

.PHONY: $(PHONY)
