
VERSION_MAJOR   := 0
VERSION_MINOR   := 1
PATCH_NUMBER    := 0
CODE_NAME       = angel_attack

DEFAULT_ARCH    := x86
ARCH            ?= $(DEFAULT_ARCH)
IS_CROSS_COMP   ?= 1

HOST_CXX        ?= g++
HOST_CC         ?= gcc
CROSS_CXX       ?= 
CROSS_CC        ?= 

CFLAGS          ?= 
LDFLAGS         ?= 
C_DEBUG_FLAGS   := -g
C_RELEASE_FLAGS := -O2

ifeq ($(IS_CROSS_COMP), 1)
	ifeq ($(CROSS_CC),)
$(error Cross compiling but no CROSS_CC was provided)
	endif
    CC := $(CROSS_CC)
else
    CC := $(HOST_CC)
endif

export CC

MAKEFILE_PATH  := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BIN_NAME       ?= dxgmx
FULL_BIN_NAME  ?= $(BIN_NAME)-$(VERSION_MAJOR).$(VERSION_MINOR).$(PATCH_NUMBER)
INCLUDE_DIR    ?= $(MAKEFILE_PATH)/include
BUILD_DIR      ?= $(MAKEFILE_PATH)/build
OBJS_DIR       ?= $(BUILD_DIR)/objs
SYSROOT_DIR    ?= $(BUILD_DIR)/sysroot/
SCRIPTS_DIR    ?= $(MAKEFILE_PATH)/scripts
OUT_ISO_PATH   ?= $(BUILD_DIR)/$(FULL_BIN_NAME).iso

OUTPUT_FORMATTED = $(SCRIPTS_DIR)/output-formatted.sh

export OUTPUT_FORMATTED
export SYSROOT_DIR
export SCRIPTS_DIR
export BUILD_DIR
export OBJS_DIR

DEFS ?= 
DEFS += \
-D__K_VER_MAJ__=$(VERSION_MAJOR) \
-D__K_VER_MIN__=$(VERSION_MINOR) \
-D__K_PATCH_N__=$(PATCH_NUMBER) \
-D__K_CODENAME__='"$(CODE_NAME)"'

SRCARCH    := $(shell $(SCRIPTS_DIR)/arch-to-srcarch.sh $(ARCH))
ifeq ($(SRCARCH), undefined)
$(error Unsupported arch $(ARCH))
endif

ifeq ($(SRCARCH), x86)
	DEFS += -D__X86__
endif

CFLAGS += \
-ffreestanding -Wall -Wextra \
-isystem=/usr/include --sysroot=$(SYSROOT_DIR) \
$(DEFS)

export CFLAGS
export DEFS

LDFLAGS    += -nostdlib -lgcc
MAKEFLAGS  += --no-print-directory
SUBDIRS    ?= init arch

-include arch/$(SRCARCH)/video/Makefile.config
include arch/$(SRCARCH)/core/Makefile.config
include arch/$(SRCARCH)/boot/Makefile.config
include init/Makefile.config

OBJS := $(addprefix $(OBJS_DIR)/, \
$(VIDEO_OBJS) \
$(ARCH_BOOT_OBJS) \
$(INIT_OBJS) \
$(ARCH_CORE_OBJS) \
)

.PHONY: $(BIN_NAME) clean mrclean iso iso-run headers
.SUFFIXES: .o .c .s

$(BIN_NAME): Makefile
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(SYSROOT_DIR)/boot/grub $(SYSROOT_DIR)/usr/include

	make headers

	@for f in $(SUBDIRS); do \
		if [ "$${f}" == "arch" ]; then \
			$(MAKE) $(MAKEFLAGS) -C $${f}/$(SRCARCH); \
		else \
			$(MAKE) $(MAKEFLAGS) -C $${f}; \
		fi \
	done

	@$(OUTPUT_FORMATTED) LD $(FULL_BIN_NAME)
	@$(CC) \
	-T arch/$(SRCARCH)/boot/linker.ld \
	$(OBJS) $(CFLAGS) $(LDFLAGS) \
	-o $(SYSROOT_DIR)/boot/$(FULL_BIN_NAME)

iso:
	make $(BIN_NAME)
	
	@$(SCRIPTS_DIR)/iso.sh \
	--sysrot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(OUT_ISO_PATH)

iso-run:
	make iso

	@$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(BUILD_DIR)/$(FULL_BIN_NAME).iso \
	--arch $(ARCH)

headers:
	@cp -r $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

clean:
	@for f in $(SUBDIRS); do \
		if [ "$${f}" == "arch" ]; then \
			$(MAKE) clean $(MAKEFLAGS) -C $${f}/$(SRCARCH); \
		else \
			$(MAKE) clean $(MAKEFLAGS) -C $${f}; \
		fi \
	done

mrclean:
	make clean
	@#check if SYSROOT_DIR doesn't actually expand to the host sysroot before removing it :-) kms
	@if [ "$(realpath --canonicalize-missing -s $(SYSROOT_DIR))" != "/" ]; then \
		rm -rf $(SYSROOT_DIR)/* ; \
	fi
