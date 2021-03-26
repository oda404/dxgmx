
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

OUTPUT_FORMATED = $(SCRIPTS_DIR)/output-formated.sh

export OUTPUT_FORMATED
export SYSROOT_DIR
export SCRIPTS_DIR
export BUILD_DIR
export OBJS_DIR

DEFINES ?= 
DEFINES += \
-D__K_VER_MAJ__=$(VERSION_MAJOR) \
-D__K_VER_MIN__=$(VERSION_MINOR) \
-D__K_PATCH_N__=$(PATCH_NUMBER) \
-D__K_CODENAME__='"$(CODE_NAME)"'

CFLAGS += \
-ffreestanding -Wall -Wextra \
-isystem=/usr/include --sysroot=$(SYSROOT_DIR) \
$(DEFINES)

export CFLAGS
export DEFINES

LDFLAGS        += -nostdlib -lgcc
MAKEFLAGS      += --no-print-directory
SUBDIRS        ?= init arch

ifeq ($(ARCH), x86)
	SRCARCH := x86
endif
ifeq ($(ARCH), i386)
	SRCARCH := x86
endif
ifeq ($(ARCH), i686)
	SRCARCH := x86
endif

-include arch/$(SRCARCH)/video/Makefile.config
include arch/$(SRCARCH)/boot/Makefile.config
include init/Makefile.config

OBJS := $(addprefix $(OBJS_DIR)/, \
$(VIDEO_OBJS) \
$(ARCH_BOOT_OBJS) \
$(INIT_OBJS) \
)

.PHONY: $(BIN_NAME) clean mrclean iso iso-qemu headers
.SUFFIXES: .o .c .s

$(BIN_NAME): Makefile
	$(shell $(SCRIPTS_DIR)/sysroot.sh $(SYSROOT_DIR))

	make headers

	@for f in $(SUBDIRS); do \
		if [ "$${f}" == "arch" ]; then \
			$(MAKE) $(MAKEFLAGS) -C $${f}/$(SRCARCH); \
		else \
			$(MAKE) $(MAKEFLAGS) -C $${f}; \
		fi \
	done

	@echo "$(shell $(OUTPUT_FORMATED) LD $(FULL_BIN_NAME))"
	@$(CC) -T arch/$(SRCARCH)/boot/linker.ld $(OBJS) $(CFLAGS) $(LDFLAGS) -o $(SYSROOT_DIR)/boot/$(FULL_BIN_NAME)

	@grub-file --is-x86-multiboot $(SYSROOT_DIR)/boot/$(FULL_BIN_NAME)

iso:
	make $(BIN_NAME)
	$(shell $(SCRIPTS_DIR)/iso.sh $(SYSROOT_DIR) $(FULL_BIN_NAME))

	#@echo "menuentry '$(FULL_BIN_NAME)' { multiboot /boot/$(FULL_BIN_NAME) }" > $(ISO_DIR)/boot/grub/grub.cfg
	#grub-mkrescue -o $(BUILD_DIR)/$(FULL_BIN_NAME).iso $(ISO_DIR)

iso-qemu:
	make iso
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/$(FULL_BIN_NAME).iso

headers:
	cp -r $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

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
	rm -f $(SYSROOT_DIR)/boot/$(FULL_BIN_NAME)
	rm -rf $(ISO_DIR)/*
	rm -f $(BUILD_DIR)/$(FULL_BIN_NAME).iso
	rm -rf $(SYSROOT_DIR)/usr/include/*
