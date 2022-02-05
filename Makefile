# Copyright 2022 Alexandru Olaru.
# Distributed under the MIT license.

APPNAME           := dxgmx

VER_MAJ           := 0
VER_MIN           := 9
PATCH_N           := 5
CODENAME          := angel_attack

ifdef BUILDCONFIG
-include $(BUILDCONFIG)
else
-include buildconfig
BUILDCONFIG := buildconfig
endif

ifdef BUILDTARGET
-include $(BUILDTARGET)
endif

ifeq ($(MAKECMDGOALS),)
    ifneq ($(HAS_BUILDCONFIG), 1)
        $(error No buildconfig file was found. See docs/buildsystem.md)
    endif
    ifneq ($(HAS_BUILDTARGET), 1)
        $(warning Building without a *.buildtarget file. See docs/buildsystem.md)
    endif
endif

# TODO: validate buildconfig and buildtarget ??

### DIRECTORIES ###
INCLUDEDIR        := include
BUILDDIR          := build
SYSROOTDIR        := $(BUILDDIR)/sysroot
SCRIPTSDIR        := scripts

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

### BASE FLAGS ###
CFLAGS            := \
-MD -MP -isystem=/usr/include -std=c2x                   \
--sysroot=$(SYSROOTDIR) -fno-omit-frame-pointer          \
-ffreestanding -fno-builtin -I$(INCLUDEDIR) \

CXXFLAGS          := $(CFLAGS)

LDFLAGS           := -nostdlib

MACROS            := \
-D_DXGMX_ -D__dxgmx__ -DDXGMX_VER_MAJ=$(VER_MAJ)                    \
-DDXGMX_VER_MIN=$(VER_MIN) -DDXGMX_PATCH_N=$(PATCH_N) \
-DDXGMX_CODENAME='"$(CODENAME)"'           \

WARNINGS          := -Wall -Wextra -Wshadow          \
-Werror-implicit-function-declaration \

### CONFIGURATION ###

include $(SCRIPTSDIR)/configparser.mk

ifeq ($(SRCARCH),x86)
	CFLAGS += -march=$(ARCH) -m32
	MACROS += -D_X86_
endif

ifeq ($(LLVM),1)
	CFLAGS += --target=$(TARGET_TRIPLET)
endif

CXXFLAGS          += $(EXTRA_CXXFLAGS) $(WARNINGS) $(MACROS)
CFLAGS            += $(EXTRA_CFLAGS) $(WARNINGS) $(MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS)

# At this point CFLAGS, CXXFLAGS and LDFLAGS should be in their final forms.

MAKEFLAGS         += --no-print-directory

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
HEADERS           :=

# The above variables will be populated recursively by
# these included makefiles.
include $(ARCH_SRCDIR)/Makefile
include $(INIT_SRCDIR)/Makefile
include $(KERNEL_SRCDIR)/Makefile
include $(INCLUDEDIR)/Makefile

COBJS             := $(filter %.c, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
COBJS             := $(COBJS:%.c=%_$(BUILDTARGET_NAME).c.o)

CXXOBJS           := $(filter %.cpp, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
CXXOBJS           := $(CXXOBJS:%.cpp=%_$(BUILDTARGET_NAME).cpp.o)

ASMOBJS           := $(filter %.S, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(BUILDTARGET_NAME).S.o)

COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CXXOBJS           := $(addprefix $(BUILDDIR)/, $(CXXOBJS))
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))

CDEPS             := $(COBJS:%.o=%.d)
CXXDEPS           := $(CXXOBJS:%.o=%.d)
ASMDEPS           := $(ASMOBJS:%.o=%.d)

SYSROOT_DIRS      := \
$(SYSROOTDIR) $(SYSROOTDIR)/boot \

SYSROOT_HEADERS   := $(HEADERS:$(INCLUDEDIR)/%=$(SYSROOTDIR)/usr/include/%)

DXGMX_DEPS        := $(SYSROOT_DIRS) \
$(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDSCRIPT)

DXGMX_COMMON_DEPS := Makefile $(BUILDCONFIG) $(BUILDTARGET)

PHONY             :=

PHONY += all
all: $(BIN_PATH)

$(BIN_PATH): $(DXGMX_DEPS) $(DXGMX_COMMON_DEPS)
	@$(OUTPUT_FORMATTED) LD $(notdir $(BIN_NAME))
	@$(LD) -T $(LDSCRIPT) $(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDFLAGS) -o $(BIN_PATH)

	@[ -f build/image.img ] || $(SCRIPTSDIR)/create-disk.sh -p build/image.img -s 128M
	@NM=$(NM) OBJCOPY=$(OBJCOPY) $(SCRIPTSDIR)/bake_symbols.sh $(BIN_PATH) 

	@cp $(BIN_PATH) $(SYSROOTDIR)/boot/

-include $(CDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(CXXDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).cpp.o: %.cpp $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CXX $<
	@$(CXX) -c $< $(CXXFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

$(SYSROOT_DIRS):
	@mkdir -p $(SYSROOT_DIRS)

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
	DXGMX_DISK=build/image.img DXGMX_MEM=128M \
		$(SCRIPTSDIR)/run.sh -i $(ISO_PATH) -a $(ARCH)

PHONY += run 
run:
	$(MAKE)
	DXGMX_DISK=build/image.img DXGMX_MEM=128M \
		$(SCRIPTSDIR)/run.sh -k $(BIN_PATH) -a $(ARCH)

PHONY += clean 
clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

.PHONY: $(PHONY)
