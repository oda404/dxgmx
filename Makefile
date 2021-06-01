
VER_MAJ           := 0
VER_MIN           := 7
PATCH_N           := 13
CODE_NAME         := angel_attack

DEFAULT_ARCH      := x86
ARCH              ?= $(DEFAULT_ARCH)
IS_CROSS_COMP     ?= 1

HOST_CC           ?= gcc
CROSS_CC          ?= 
HOST_AS           ?= as
CROSS_AS          ?= 
HOST_LD           ?= ld
CROSS_LD          ?= 
HOST_AR           ?= ar
CROSS_AR          ?= 

CFLAGS            ?= 
LDFLAGS           ?= 
CFLAGS_DEBUG      := -g
CFLAGS_RELEASE    := -O2
PHONY_TARGETS     :=
DXGMX_DEPS        := 

# if we are cross compiling check if CROSS_CC was exported
ifeq ($(IS_CROSS_COMP), 1)
	LIBC_FREESTANDING := 1
	ifeq ($(CROSS_CC),)
$(error Cross compiling but no CROSS_CC was exported)
	endif

    CC := $(CROSS_CC)
else
    CC := $(HOST_CC)
	AS := $(HOST_AS)
	LD := $(HOST_LD)
	AR := $(HOST_AR)
endif

export CC
export AS
export LD
export AR

# set misc directories
INCLUDE_DIR    := include
BUILD_DIR      := build
SYSROOT_DIR    := $(BUILD_DIR)/sysroot
SCRIPTS_DIR    := scripts

export INCLUDE_DIR
export SYSROOT_DIR
export SCRIPTS_DIR
export BUILD_DIR

# check if the arch is supported and set SRCARCH
SRCARCH := $(shell $(SCRIPTS_DIR)/arch-to-srcarch.sh $(ARCH))
ifeq ($(SRCARCH), undefined)
$(error Unsupported arch $(ARCH))
endif

# set src directories
ARCH_DIR         := arch/$(SRCARCH)
ARCH_INCLUDE_DIR := $(ARCH_DIR)/include
INIT_DIR         := init
KERNEL_DIR       := kernel

export ARCH_DIR
export INIT_DIR

FULL_BIN_NAME  ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
FULL_BIN_PATH  ?= $(FULL_BIN_NAME)
ISO_PATH       ?= $(FULL_BIN_NAME).iso

# format the output
OUTPUT_FORMATTED = $(SCRIPTS_DIR)/output-formatted.sh
export OUTPUT_FORMATTED

DEFS ?= 
DEFS += \
-D__DXGMX__ \
-D__KVER_MAJ__=$(VER_MAJ) \
-D__KVER_MIN__=$(VER_MIN) \
-D__KPATCH_N__=$(PATCH_N) \
-D__KCODENAME__='"$(CODE_NAME)"' 

# set SRCARCH macro
ifeq ($(SRCARCH), x86)
	DEFS += -D__X86__
endif

WARNINGS := \
-Werror-implicit-function-declaration \

CFLAGS += \
-ffreestanding -Wall -Wextra \
-isystem=/usr/include --sysroot=$(SYSROOT_DIR) \
$(DEFS) $(WARNINGS) -MD -MP

export CFLAGS
export DEFS

LDFLAGS += \
-nostdlib -lgcc \

MAKEFLAGS  += --no-print-directory

OBJS_ARCH    := 
OBJS_INIT    := 
OBJS_KERNEL  :=

# set objs recursively
include $(ARCH_DIR)/Makefile
include $(INIT_DIR)/Makefile
include $(KERNEL_DIR)/Makefile

OBJS            := \
$(OBJS_ARCH) $(OBJS_INIT) \
$(OBJS_KERNEL)

# prefix objs with the build dir path
DEPS            := $(addprefix $(BUILD_DIR)/, $(OBJS))
# change their extensions to .d
DEPS            := $(patsubst %.o, %.d, $(DEPS))

DXGMX_DEPS += builddir sysroot kernel_headers \
arch_headers $(addprefix $(BUILD_DIR)/, $(OBJS)) \
Makefile 

PHONY_TARGETS += all dxgmx 
all: dxgmx
dxgmx: $(DXGMX_DEPS)
	@$(OUTPUT_FORMATTED) LD $(notdir $@)

	@$(CC) -T arch/$(SRCARCH)/boot/linker.ld \
	$(addprefix $(BUILD_DIR)/, $(OBJS)) \
	$(LDFLAGS) -o $(FULL_BIN_PATH)

	@cp $(FULL_BIN_PATH) $(SYSROOT_DIR)/boot/

-include $(DEPS)

# deps are stripped of the BUILD_DIR
$(BUILD_DIR)/%.o : $(patsubst $(BUILD_DIR), ,%.c) Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

# deps are stripped of the BUILD_DIR
$(BUILD_DIR)/%.o : $(patsubst $(BUILD_DIR), ,%.S) Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(CC) -c $< $(CFLAGS) -o $@

PHONY_TARGETS += builddir 
# create the build dir
builddir:
	@mkdir -p $(BUILD_DIR)

PHONY_TARGETS += sysroot 
# create the sysroot dir
sysroot:
	@mkdir -p $(SYSROOT_DIR)
	@mkdir -p $(SYSROOT_DIR)/boot
	@mkdir -p $(SYSROOT_DIR)/usr/include/dxgmx/$(SRCARCH)

PHONY_TARGETS += kernel_headers 
# kernel headers 
kernel_headers: 
	@cp -ru $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

PHONY_TARGETS += arch_headers 
# arch specific headers
arch_headers:
	@cp -ru $(ARCH_DIR)/include/* \
	$(SYSROOT_DIR)/usr/include/dxgmx/$(SRCARCH)

PHONY_TARGETS += iso 
iso:
	$(MAKE)
	$(SCRIPTS_DIR)/iso.sh \
	--sysroot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(ISO_PATH) \
	--boot-spec multiboot

PHONY_TARGETS += iso-run 
iso-run:
	$(MAKE) iso
	$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(ISO_PATH) \
	--arch $(ARCH)

PHONY_TARGETS += run 
run:
	$(MAKE)
	$(SCRIPTS_DIR)/run.sh \
	--kernel-path $(FULL_BIN_PATH) \
	--arch $(ARCH)

PHONY_TARGETS += clean 
clean:
	@rm -f $(addprefix $(BUILD_DIR)/, $(OBJS))
	@rm -f $(DEPS)

PHONY_TARGETS += mrclean 
mrclean:
	$(MAKE) clean $(KILL_STDOUT)
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rm -f $(BUILD_DIR)

.PHONY: $(PHONY_TARGETS)
