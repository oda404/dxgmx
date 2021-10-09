# Copyright 2021 Alexandru Olaru.
# Distributed under the MIT license.

APPNAME           := dxgmx

VER_MAJ           := 0
VER_MIN           := 8
PATCH_N           := 6
CODENAME          := angel_attack

ifdef BUILDCONFIG
-include $(BUILDCONFIG)
else
-include buildconfig
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
-MD -MP -m32 -isystem=/usr/include               \
--sysroot=$(SYSROOTDIR) -fno-omit-frame-pointer  \
-ffreestanding -fno-builtin $(BT_CFLAGS)         \

CXXFLAGS          := $(CFLAGS) $(BT_CXXFLAGS)

LDFLAGS           := -nostdlib -lgcc $(BT_LDFLAGS)

MACROS            := \
-D_DXGMX_ -D_DXGMX_VER_MAJ_=$(VER_MAJ)                    \
-D_DXGMX_VER_MIN_=$(VER_MIN) -D_DXGMX_PATCH_N_=$(PATCH_N) \
-D_DXGMX_CODENAME_='"$(CODENAME)"' $(BT_MACROS)           \

WARNINGS          := -Wall -Wextra                   \
-Werror-implicit-function-declaration $(BT_WARNINGS) \

### CONFIGURATION ###

ifeq ($(shell test '$(CONFIG_OPTIMIZATIONS)' -eq '$(CONFIG_OPTIMIZATIONS)' 2> /dev/null && echo 0 || echo 1),0)
	CFLAGS += -O$(CONFIG_OPTIMIZATIONS)
	CXXFLAGS += -O$(CONFIG_OPTIMIZATIONS)
endif

ifeq ($(CONFIG_DEBUG_SYMS),1)
	CFLAGS += -g
	CXXFLAGS += -g
endif

ifeq ($(shell test '$(CONFIG_LOGLVL)' -eq '$(CONFIG_LOGLVL)' 2> /dev/null && echo 0 || echo 1),0)
	MACROS += -D_DXGMX_LOGLVL_=$(CONFIG_LOGLVL)
endif

ifeq ($(CONFIG_STACK_PROT),none)
	CFLAGS += -fno-stack-protector
else ifeq ($(CONFIG_STACK_PROT),normal)
	CFLAGS += -fstack-protector
else ifeq ($(CONFIG_STACK_PROT),strong)
	CFLAGS += -fstack-protector-strong
else ifeq ($(CONFIG_STACK_PROT),all)
	CFLAGS += -fstack-protector-all
endif

ifeq ($(SRCARCH),x86)
	# assume i686 if building for x86
	CFLAGS += -march=i686
	MACROS += -D_X86_
endif

### CONFIGURATION END ###

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
COBJS             := $(COBJS:%.c=%_$(BT_NAME).c.o)

CXXOBJS           := $(filter %.cpp, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
CXXOBJS           := $(CXXOBJS:%.cpp=%_$(BT_NAME).cpp.o)

ASMOBJS           := $(filter %.S, $(ARCH_SRC) $(INIT_SRC) $(KERNEL_SRC))
ASMOBJS           := $(ASMOBJS:%.S=%_$(BT_NAME).S.o)

COBJS             := $(addprefix $(BUILDDIR)/, $(COBJS))
CXXOBJS           := $(addprefix $(BUILDDIR)/, $(CXXOBJS))
ASMOBJS           := $(addprefix $(BUILDDIR)/, $(ASMOBJS))

CDEPS             := $(COBJS:%.o=%.d)
CXXDEPS           := $(CXXOBJS:%.o=%.d)
ASMDEPS           := $(ASMOBJS:%.o=%.d)

SYSROOT_DIRS      := \
$(SYSROOTDIR) $(SYSROOTDIR)/boot \

SYSROOT_HEADERS   := $(HEADERS:$(INCLUDEDIR)/%=$(SYSROOTDIR)/usr/include/%)

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
$(BUILDDIR)/%_$(BT_NAME).c.o: %.c Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

-include $(CXXDEPS)
$(BUILDDIR)/%_$(BT_NAME).cpp.o: %.cpp Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CXX $<
	@$(CXX) -c $< $(CXXFLAGS) -o $@

-include $(ASMDEPS)
$(BUILDDIR)/%_$(BT_NAME).S.o: %.S Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(AS) -c $< $(CFLAGS) -o $@

$(SYSROOT_DIRS):
	@mkdir -p $(SYSROOT_DIRS)

$(SYSROOTDIR)/usr/include/%.h: $(INCLUDEDIR)/%.h
	@mkdir -p $(dir $@) 2> /dev/null || true
	@cp -ru $< $@

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
	qemu-system-x86_64 --enable-kvm -m 2G -cpu host,migratable=off -cdrom $(ISO_PATH)

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
	@rm -r $(BUILDDIR) 2> /dev/null || true

.PHONY: $(PHONY)
