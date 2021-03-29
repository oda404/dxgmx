
VERSION_MAJOR   := 0
VERSION_MINOR   := 1
PATCH_NUMBER    := 0
CODE_NAME       := angel_attack

DEFAULT_ARCH    := x86
ARCH            ?= $(DEFAULT_ARCH)
IS_CROSS_COMP   ?= 1

HOST_CC         ?= gcc
CROSS_CC        ?= 

CFLAGS          ?= 
LDFLAGS         ?= 
C_DEBUG_FLAGS   := -g
C_RELEASE_FLAGS := -O2

# if we are cross compiling check if CROSS_CC was exported
ifeq ($(IS_CROSS_COMP), 1)
	ifeq ($(CROSS_CC),)
$(error Cross compiling but no CROSS_CC was exported)
	endif
    CC := $(CROSS_CC)
else
    CC := $(HOST_CC)
endif

export CC

# declare some handy paths
MAKEFILE_PATH   := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BIN_NAME        ?= dxgmx
FULL_BIN_NAME   ?= $(BIN_NAME)-$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCH_NUMBER)
INCLUDE_DIR     ?= $(MAKEFILE_PATH)/include
BUILD_DIR       ?= $(MAKEFILE_PATH)/build
OBJS_DIR        ?= $(BUILD_DIR)/objs
SYSROOT_DIR     ?= $(BUILD_DIR)/sysroot/
SCRIPTS_DIR     ?= $(MAKEFILE_PATH)/scripts
OUT_ISO_PATH    ?= $(BUILD_DIR)/$(FULL_BIN_NAME).iso
LIBC_DIR        ?= $(MAKEFILE_PATH)/libc
KLIBS_DIR        ?= $(BUILD_DIR)/static

# format the output
OUTPUT_FORMATTED = $(SCRIPTS_DIR)/output-formatted.sh

export OUTPUT_FORMATTED
export SYSROOT_DIR
export SCRIPTS_DIR
export BUILD_DIR
export OBJS_DIR
export KLIBS_DIR

# check if the arch is supported
SRCARCH := $(shell $(SCRIPTS_DIR)/arch-to-srcarch.sh $(ARCH))
ifeq ($(SRCARCH), undefined)
$(error Unsupported arch $(ARCH))
endif

DEFS ?= 
DEFS += \
-D__K_VER_MAJ__=$(VERSION_MAJOR) \
-D__K_VER_MIN__=$(VERSION_MINOR) \
-D__K_PATCH_N__=$(PATCH_NUMBER) \
-D__K_CODENAME__='"$(CODE_NAME)"'

ifeq ($(SRCARCH), x86)
	DEFS += -D__X86__
endif

# the kernel expects entry.o to be present in  
# $(BUILD_DIR)/objs as well as the following static libs
# in $(BUILD_DIR)/static by the time it's ready to link:
# init.a
# arch.a
# libc.a

KLIBS := \
$(KLIBS_DIR)/init.a \
$(KLIBS_DIR)/arch.a \
$(KLIBS_DIR)/libc.a 

SUBDIRS := \
init arch libc

CFLAGS += \
-ffreestanding -Wall -Wextra \
-isystem=/usr/include --sysroot=$(SYSROOT_DIR) \
$(DEFS)

export CFLAGS
export DEFS

LDFLAGS    += \
-nostdlib -lgcc \
-L$(KLIBS_DIR)  -l:init.a -l:arch.a -lc \

MAKEFLAGS  += --no-print-directory

.PHONY: FORCE $(BIN_NAME) clean mrclean iso iso-run headers build-dir libc

$(BIN_NAME): build-dir headers $(KLIBS) Makefile

	@$(OUTPUT_FORMATTED) LD $(FULL_BIN_NAME)
	@$(CC) \
	-T arch/$(SRCARCH)/boot/linker.ld \
	$(OBJS_DIR)/entry.o $(CFLAGS) $(LDFLAGS) \
	-o $(SYSROOT_DIR)/boot/$(FULL_BIN_NAME)

$(KLIBS_DIR)/libc.a: FORCE
	@LIBC_FREESTANDING=1 $(MAKE) -C $(LIBC_DIR)

$(KLIBS_DIR)/arch.a: FORCE
	@$(MAKE) -C arch/$(SRCARCH)

%.a: FORCE
	@$(MAKE) -C $(basename $(notdir $@))

iso:
	make $(BIN_NAME)
	
	@$(SCRIPTS_DIR)/iso.sh \
	--sysroot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(OUT_ISO_PATH)

iso-run:
	make iso

	@$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(BUILD_DIR)/$(FULL_BIN_NAME).iso \
	--arch $(ARCH)

build-dir:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(KLIBS_DIR)
	@mkdir -p $(OBJS_DIR)

headers:
	@mkdir -p $(SYSROOT_DIR)
	@mkdir -p $(SYSROOT_DIR)/usr/include
	@mkdir -p $(SYSROOT_DIR)/boot/grub
	@cp -r $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

clean:
	@$(RM) -rf $(OBJS_DIR)/*
	@$(RM) -f $(KLIBS)

mrclean:
	make clean
	$(RM) -f $(OUT_ISO_PATH)
	@#check if SYSROOT_DIR doesn't actually expand to the host sysroot before removing it :-) kms
	@if [ "$(realpath --canonicalize-missing -s $(SYSROOT_DIR))" != "/" ]; then \
		rm -rf $(SYSROOT_DIR)/* ; \
	fi
