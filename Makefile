
VER_MAJ           := 0
VER_MIN           := 2
PATCH_N           := 1
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

LIBC_FREESTANDING := 0
BOOT_SPEC         ?= multiboot2

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

# declare some handy paths
FULL_BIN_NAME   ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
INCLUDE_DIR     ?= $(PWD)/include
BUILD_DIR       ?= $(PWD)/build
SYSROOT_DIR     ?= $(BUILD_DIR)/sysroot
SCRIPTS_DIR     ?= $(PWD)/scripts
FULL_BIN_PATH   ?= $(FULL_BIN_NAME)
OUT_ISO_PATH    ?= $(BUILD_DIR)/$(FULL_BIN_NAME).iso

# format the output
OUTPUT_FORMATTED = $(SCRIPTS_DIR)/output-formatted.sh

export OUTPUT_FORMATTED
export SYSROOT_DIR
export SCRIPTS_DIR
export BUILD_DIR

# check if the arch is supported
SRCARCH := $(shell $(SCRIPTS_DIR)/arch-to-srcarch.sh $(ARCH))
ifeq ($(SRCARCH), undefined)
$(error Unsupported arch $(ARCH))
endif

DEFS ?= 
DEFS += \
-D__DXGMX__ \
-D__KVER_MAJ__=$(VER_MAJ) \
-D__KVER_MIN__=$(VER_MIN) \
-D__KPATCH_N__=$(PATCH_N) \
-D__KCODENAME__='"$(CODE_NAME)"' 

ifeq ($(BOOT_SPEC), multiboot2)
	DEFS += -D__MBOOT2__
endif
ifeq ($(BOOT_SPEC), multiboot)
	DEFS += -D__MBOOT__
endif
ifeq ($(BOOT_SPEC), standalone)
	DEFS += -D__STANDALONE_BOOT__
endif

ifeq ($(SRCARCH), x86)
	DEFS += -D__X86__
endif

CFLAGS += \
-ffreestanding -Wall -Wextra \
-isystem=/usr/include --sysroot=$(SYSROOT_DIR) \
$(DEFS) -MMD -MP

export CFLAGS
export DEFS

LDFLAGS += \
-nostdlib -lgcc \

MAKEFLAGS  += --no-print-directory

OBJS_ARCH := 
OBJS_INIT := 
OBJS_LIBC := 

INC_MKFILE_DIR := arch/$(SRCARCH)
include arch/$(SRCARCH)/Makefile
INC_MKFILE_DIR := init
include init/Makefile
INC_MKFILE_DIR := libc
include libc/Makefile.config

OBJS_ARCH := $(addprefix arch/$(SRCARCH)/, $(OBJS_ARCH))
OBJS_INIT := $(addprefix init/, $(OBJS_INIT))

OBJS := $(OBJS_ARCH) $(OBJS_INIT)

DEPS := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEPS := $(patsubst %.o, %.d, $(DEPS))

.PHONY: FORCE all dxgmx clean mrclean \
iso iso-run sysroot-struct builddir-struct libc headers

all: dxgmx

dxgmx: builddir-struct sysroot-struct headers libc $(addprefix $(BUILD_DIR)/, $(OBJS)) Makefile
	@$(OUTPUT_FORMATTED) LD $(notdir $@)

	@$(CC) -T arch/$(SRCARCH)/boot/linker.ld \
	$(addprefix $(BUILD_DIR)/, $(OBJS) $(OBJS_LIBC)) \
	$(LDFLAGS) -o $(FULL_BIN_PATH)

	@cp $(FULL_BIN_PATH) $(SYSROOT_DIR)/boot/

-include $(DEPS)

$(BUILD_DIR)/%.o : $(patsubst $(BUILD_DIR), ,%.c) Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

$(BUILD_DIR)/%.o : $(patsubst $(BUILD_DIR), ,%.S) Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(CC) -c $< $(CFLAGS) -o $@

builddir-struct:
	@mkdir -p $(BUILD_DIR)

sysroot-struct:
	@mkdir -p $(SYSROOT_DIR)
	@mkdir -p $(SYSROOT_DIR)/boot
	@mkdir -p $(SYSROOT_DIR)/usr/include

# the include path in a freestanding env is 
# in $(SYSROOT_DIR)/usr/include so we copy everything there 
headers:
	@cp -r $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

libc:
	@LIBC_FREESTANDING=$(LIBC_FREESTANDING) $(MAKE) -C libc/

iso:
	$(MAKE)
	$(SCRIPTS_DIR)/iso.sh \
	--sysroot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(OUT_ISO_PATH) \
	--boot-spec $(BOOT_SPEC)

iso-run:
	$(MAKE) iso
	$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(OUT_ISO_PATH) \
	--arch $(ARCH)

clean:
	@rm -rf $(dir $(addprefix $(BUILD_DIR)/, $(OBJS)))
	@rm -rf $(BUILD_DIR)/arch
	@rm -f $(FULL_BIN_PATH)
	$(MAKE) clean -C libc

mrclean:
	$(MAKE) clean $(KILL_STDOUT)
	@rm -rf $(BUILD_DIR)
