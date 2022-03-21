# Copyright 2021-2022 Alexandru Olaru.
# Distributed under the MIT license.

KERNEL_NAME       := dxgmx

VER_MAJ           := 0
VER_MIN           := 9
PATCH_N           := 10
CODENAME          := angel_attack

# Try to include the buildconfig
ifdef BUILDCONFIG
    -include $(BUILDCONFIG)
else
    -include buildconfig
    BUILDCONFIG := buildconfig
endif

# Try to include the buildtarget
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

### MISC DIRECTORIES ###
BUILDDIR          := build/
SYSROOTDIR        := $(BUILDDIR)/slash/
SCRIPTSDIR        := scripts/

ARCH              := $(shell $(SCRIPTSDIR)/arch.sh --from-target-trip $(TARGET_TRIPLET))
ifeq ($(ARCH), undefined)
    $(error Couldn't get arch from target triplet: '$(TARGET_TRIPLET'))
endif

SRCARCH           := $(shell $(SCRIPTSDIR)/arch.sh --to-srcarch $(ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch: '$(ARCH)')
endif

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
-ffreestanding -fno-builtin -I$(INCLUDE_SRCDIR)

CXXFLAGS          := $(CFLAGS)

LDFLAGS           := -nostdlib

MACROS            := \
-D_DXGMX_ -DDXGMX_VER_MAJ=$(VER_MAJ) \
-DDXGMX_VER_MIN=$(VER_MIN) -DDXGMX_PATCH_N=$(PATCH_N) \
-DDXGMX_CODENAME='"$(CODENAME)"' 

WARNINGS          := -Wall -Wextra -Wshadow \
-Werror-implicit-function-declaration \
-Wunused -Wnull-dereference -Wdouble-promotion \
-Wformat=2 -Wmisleading-indentation #-Wsign-conversion

### CONFIGURATION ###

include $(SCRIPTSDIR)/configparser.mk

ifeq ($(SRCARCH),x86)
	CFLAGS += -march=$(ARCH) -m32
	MACROS += -D_X86_
endif

ifeq ($(LLVM),1)
	CFLAGS += --target=$(TARGET_TRIPLET)
endif

CFLAGS            += $(EXTRA_CFLAGS) $(WARNINGS) $(MACROS)
CXXFLAGS          += $(EXTRA_CXXFLAGS) $(WARNINGS) $(MACROS)
LDFLAGS           += $(EXTRA_LDFLAGS)

# At this point CFLAGS, CXXFLAGS and LDFLAGS should be in their final forms.

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

# The above variables will be populated recursively by
# these included makefiles.
include $(ARCH_SRCDIR)/Makefile
include $(INIT_SRCDIR)/Makefile
include $(KERNEL_SRCDIR)/Makefile
#include $(FS_SRCDIR)/Makefile
include $(INCLUDE_SRCDIR)/Makefile

ALL_SRC := $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC) $(FS_SRC)

# Filter out and add TARGET_NAME to each object.
COBJS             := $(filter %.c, $(ALL_SRC))
COBJS             := $(COBJS:%.c=%_$(BUILDTARGET_NAME).c.o)

CXXOBJS           := $(filter %.cpp, $(ALL_SRC))
CXXOBJS           := $(CXXOBJS:%.cpp=%_$(BUILDTARGET_NAME).cpp.o)

ASMOBJS           := $(filter %.S, $(ALL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(BUILDTARGET_NAME).S.o)

# Prefix each object with the build directory path.
COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CXXOBJS           := $(addprefix $(BUILDDIR)/, $(CXXOBJS))
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))

CDEPS             := $(COBJS:%.o=%.d)
CXXDEPS           := $(CXXOBJS:%.o=%.d)
ASMDEPS           := $(ASMOBJS:%.o=%.d)

SYSROOT_DIRS      := \
$(SYSROOTDIR) $(SYSROOTDIR)/boot \

SYSROOT_HEADERS   := $(HEADERS:$(INCLUDE_SRCDIR)/%=$(SYSROOTDIR)/usr/include/%)

DXGMX_DEPS        := $(SYSROOT_DIRS) \
$(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDSCRIPT)

DXGMX_COMMON_DEPS := Makefile $(BUILDCONFIG) $(BUILDTARGET)

PHONY             :=

PHONY += all
all: $(KERNEL_BIN_PATH)

$(KERNEL_BIN_PATH): $(DXGMX_DEPS) $(DXGMX_COMMON_DEPS)
	@$(PRETTY_PRINT) LD $(notdir $(KERNEL_BIN_NAME))
	@$(LD) -T $(LDSCRIPT) $(COBJS) $(CXXOBJS) $(ASMOBJS) $(LDFLAGS) -o $(KERNEL_BIN_PATH)

	@[ -f build/image.img ] || $(SCRIPTSDIR)/create-disk.sh -p build/image.img
	@NM=$(NM) OBJCOPY=$(OBJCOPY) $(SCRIPTSDIR)/bake_symbols.sh $(KERNEL_BIN_PATH) 

	@cp $(KERNEL_BIN_PATH) $(SYSROOTDIR)/boot/

-include $(CDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).c.o: %.c $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(CXXDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).cpp.o: %.cpp $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) CXX $<
	@$(CXX) -c $< $(CXXFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(BUILDTARGET_NAME).S.o: %.S $(DXGMX_COMMON_DEPS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

$(SYSROOT_DIRS):
	@mkdir -p $(SYSROOT_DIRS)

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
		$(SCRIPTSDIR)/run.sh -i $(KERNEL_ISO_PATH) -a $(ARCH)

PHONY += run 
run:
	$(MAKE)
	DXGMX_DISK=build/image.img DXGMX_MEM=128M \
		$(SCRIPTSDIR)/run.sh -k $(KERNEL_BIN_PATH) -a $(ARCH)

PHONY += clean 
clean:
	@rm -f $(COBJS) $(CXXOBJS) $(ASMOBJS)
	@rm -f $(CDEPS) $(CXXDEPS) $(ASMDEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -r $(BUILDDIR) 2> /dev/null || true

.PHONY: $(PHONY)
