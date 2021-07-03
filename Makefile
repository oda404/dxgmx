
APPNAME           := dxgmx

VER_MAJ           := 0
VER_MIN           := 7
PATCH_N           := 15
CODENAME          := angel_attack

-include buildconfig

# directories
INCLUDE_DIR       := include
BUILD_DIR         := build
SYSROOT_DIR       := $(BUILD_DIR)/sysroot
SCRIPTS_DIR       := scripts

TARGET_TRIPLET    ?= x86-dxgmx-elf
IS_CROSS_COMP     ?= 1

ARCH              := $(shell $(SCRIPTS_DIR)/target-triplet-to-arch.sh $(TARGET_TRIPLET))
ifeq ($(ARCH), undefined)
    $(error Couldn't get arch from target triplet)
endif

SRCARCH           := $(shell $(SCRIPTS_DIR)/arch-to-srcarch.sh $(ARCH))
ifeq ($(SRCARCH), undefined)
    $(error Unsupported arch $(ARCH))
endif

ARCH_DIR          := arch/$(SRCARCH)
ARCH_INCLUDE_DIR  := $(ARCH_DIR)/include
INIT_DIR          := init
KERNEL_DIR        := kernel

CC                ?= gcc
LD                ?= ld
AR                ?= ar
AS                ?= as

CFLAGS            := 
EXTRA_CFLAGS      ?= 
LDFLAGS           := 
EXTRA_LDFLAGS     ?= 
DEBUG_CFLAGS      := -g
RELEASE_CFLAGS    := -O2

CFLAGS            += $(EXTRA_CFLAGS)
LDFLAGS           += $(EXTRA_LDFLAGS)

FULL_BIN_NAME     ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
FULL_BIN_PATH     ?= $(FULL_BIN_NAME)
ISO_PATH          ?= $(FULL_BIN_PATH).iso

BUILD_TARGET      ?= debug
ifeq ($(BUILD_TARGET), debug)
    CFLAGS += $(DEBUG_CFLAGS)
else ifeq ($(BUILD_TARGET), release)
    CFLAGS += $(RELEASE_CFLAGS)
else
    $(error Unknown BUILD_TARGET=$(BUILD_TARGET))
endif

# format the output
OUTPUT_FORMATTED = $(SCRIPTS_DIR)/output-formatted.sh
export OUTPUT_FORMATTED

MACROS            := \
-D__dxgmx__ -D_DXGMX_ -D_DXGMX_VER_MAJ=$(VER_MAJ) \
-D_DXGMX_VER_MIN_=$(VER_MIN) -D_DXGMX_PATCH_N_=$(PATCH_N) \
-D_DXGMX_CODENAME_='"$(CODENAME)"' 

# set SRCARCH macro
ifeq ($(SRCARCH), x86)
	MACROS += -D_X86_
endif

WARNINGS          := -Wall -Wextra \
-Werror-implicit-function-declaration

CFLAGS            += -MD -MP -m32 \
-ffreestanding -isystem=/usr/include \
--sysroot=$(SYSROOT_DIR)

CFLAGS            += $(WARNINGS) $(MACROS) 

LDFLAGS           += -nostdlib 

MAKEFLAGS         += --no-print-directory

OBJS_ARCH         := 
OBJS_INIT         := 
OBJS_KERNEL       := 

# set objs recursively
include $(ARCH_DIR)/Makefile
include $(INIT_DIR)/Makefile
include $(KERNEL_DIR)/Makefile

OBJS              := $(OBJS_ARCH) $(OBJS_INIT) $(OBJS_KERNEL)
OBJS              := $(addprefix $(BUILD_DIR)/, $(OBJS))

DEPS              := $(OBJS:%.o=%.d)

SYSROOT           := \
$(SYSROOT_DIR) $(SYSROOT_DIR)/boot \
$(SYSROOT_DIR)/usr/include/dxgmx

DXGMX_DEPS        := $(SYSROOT) kernel_headers $(OBJS) Makefile

PHONY             :=

PHONY += all $(FULL_BIN_PATH) 
all: $(FULL_BIN_PATH)
$(FULL_BIN_PATH): $(DXGMX_DEPS)
	@$(OUTPUT_FORMATTED) LD $(notdir $(FULL_BIN_NAME))

	@$(CC) -T arch/$(SRCARCH)/boot/linker.ld \
	$(OBJS) $(LDFLAGS) -o $(FULL_BIN_PATH)

	@cp $(FULL_BIN_PATH) $(SYSROOT_DIR)/boot/

-include $(DEPS)

$(BUILD_DIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) CC $<
	@$(CC) -c $< $(CFLAGS) -o $@

$(BUILD_DIR)/%.o: %.S Makefile
	@mkdir -p $(dir $@)
	@$(OUTPUT_FORMATTED) AS $<
	@$(CC) -c $< $(CFLAGS) -o $@

PHONY += $(SYSROOT) 
$(SYSROOT):
	@mkdir -p $(SYSROOT)

PHONY += kernel_headers 
# kernel headers 
kernel_headers: 
	@cp -ru $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

PHONY += iso 
iso:
	$(MAKE)
	$(SCRIPTS_DIR)/iso.sh \
	--sysroot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(ISO_PATH) \
	--boot-spec multiboot

PHONY += iso-run 
iso-run:
	$(MAKE) iso
	$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(ISO_PATH) \
	--arch $(ARCH)

PHONY += run 
run:
	$(MAKE)
	$(SCRIPTS_DIR)/run.sh \
	--kernel-path $(FULL_BIN_PATH) \
	--arch $(ARCH)

PHONY += clean 
clean:
	@rm -f $(OBJS)
	@rm -f $(DEPS)

PHONY += mrclean 
mrclean:
	$(MAKE) clean
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?$$')
	@rmdir -p $$(find build -type d)

.PHONY: $(PHONY)
