
VER_MAJ           := 0
VER_MIN           := 4
PATCH_N           := 3
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

BOOT_SPEC         ?= multiboot

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

CWD := $(PWD)

# set misc directories
INCLUDE_DIR    := $(CWD)/include
BUILD_DIR      := $(CWD)/build
SYSROOT_DIR    := $(BUILD_DIR)/sysroot
SCRIPTS_DIR    := $(CWD)/scripts

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
ARCH_DIR       := $(CWD)/arch/$(SRCARCH)
INIT_DIR       := $(CWD)/init
LIBKC_DIR      := $(CWD)/libkc
CXFFXX_DIR     := $(CWD)/cxffxx
export ARCH_DIR
export INIT_DIR
export LIBKC_DIR
export CXFFXX_DIR

FULL_BIN_NAME  ?= dxgmx-$(VER_MAJ).$(VER_MIN).$(PATCH_N)
FULL_BIN_PATH  ?= $(CWD)/$(FULL_BIN_NAME)
ISO_PATH       ?= $(CWD)/$(FULL_BIN_NAME).iso

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

# set boot spec macro
ifeq ($(BOOT_SPEC), multiboot2)
	DEFS += -D__MBOOT2__
endif
ifeq ($(BOOT_SPEC), multiboot)
	DEFS += -D__MBOOT__
endif
ifeq ($(BOOT_SPEC), standalone)
	DEFS += -D__STANDALONE_BOOT__
endif

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

OBJS_ARCH  := 
OBJS_INIT  := 
OBJS_LIBKC := 

# set objs recursively
include $(ARCH_DIR)/Makefile
include $(INIT_DIR)/Makefile
include $(LIBKC_DIR)/Makefile

OBJS_ARCH       := $(addprefix arch/$(SRCARCH)/, $(OBJS_ARCH))
OBJS_INIT       := $(addprefix init/, $(OBJS_INIT))
OBJS_LIBKC      := $(addprefix libkc/, $(OBJS_LIBKC))

OBJS            := $(OBJS_ARCH) $(OBJS_INIT) $(OBJS_LIBKC)

DEPS            := $(addprefix $(BUILD_DIR)/, $(OBJS))
DEPS            := $(patsubst %.o, %.d, $(DEPS))

# set all headers recursively
HEADERS         := 
include $(INCLUDE_DIR)/Makefile
# set headers as they should be in SYSROOT_DIR/usr/include
SYSROOT_HEADERS := $(addprefix $(SYSROOT_DIR)/usr/include/, $(HEADERS))

.PHONY: all dxgmx clean mrclean \
iso iso-run run sysroot-struct builddir-struct libc

all: dxgmx

dxgmx: builddir-struct sysroot-struct $(SYSROOT_HEADERS) libc $(addprefix $(BUILD_DIR)/, $(OBJS)) Makefile
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

# create the build dirISO_PATH
builddir-struct:
	@mkdir -p $(BUILD_DIR)

# create the sysroot dir
sysroot-struct:
	@mkdir -p $(SYSROOT_DIR)
	@mkdir -p $(SYSROOT_DIR)/boot
	@mkdir -p $(SYSROOT_DIR)/usr/include

# the include path in a freestanding env is 
# in $(SYSROOT_DIR)/usr/include so we copy everything there 
$(SYSROOT_HEADERS): $(addprefix $(INCLUDE_DIR)/, $(HEADERS))
	@cp -ru $(INCLUDE_DIR)/* $(SYSROOT_DIR)/usr/include

iso:
	$(MAKE)
	$(SCRIPTS_DIR)/iso.sh \
	--sysroot-dir $(SYSROOT_DIR) \
	--bin-name $(FULL_BIN_NAME) \
	--out-iso-path $(ISO_PATH) \
	--boot-spec $(BOOT_SPEC)

iso-run:
	$(MAKE) iso
	$(SCRIPTS_DIR)/iso-run.sh \
	--iso-path $(ISO_PATH) \
	--arch $(ARCH)

run:
	$(MAKE)
	$(SCRIPTS_DIR)/run.sh \
	--kernel-path $(FULL_BIN_PATH) \
	--arch $(ARCH)

clean:
	@rm -f $(addprefix $(BUILD_DIR)/, $(OBJS))
	@rm -f $(DEPS)

mrclean:
	$(MAKE) clean $(KILL_STDOUT)
	@rm -f $$(ls | grep -Eo '^dxgmx-[0-9]+.[0-9]+.[0-9]+(.iso)?')
	@rm -f $(BUILD_DIR)
	@rm -f $(ISO_PATH)
