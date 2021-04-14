
WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CC             ?= gcc
AR             ?= ar
CFLAGS         := -Wall -Wextra -MD -MP
DEFS           := 
CFLAGS_DEBUG   := -g
CFLAGS_RELEASE := -O2

ifeq ($(LIBC_FREESTANDING), 1)
	CFLAGS += \
	-ffreestanding --sysroot=$(SYSROOT_DIR) \
	-isystem=/usr/include/

	DEFS += -D__LIBC_FREESTANDING__
else
	
endif

CFLAGS += $(DEFS)

include Makefile.config

DEPS    := $(addprefix $(BUILD_DIR)/, $(OBJS_LIBC))
DEPS    := $(patsubst %.o, %.d, $(DEPS))

.PHONY: all libc libc-builddir clean

all: libc

libc: libc-builddir $(addprefix $(BUILD_DIR)/, $(OBJS_LIBC)) Makefile

-include $(DEPS)

$(BUILD_DIR)/libc/%.o : $(patsubst $(BUILD_DIR), ,%.c)
	@$(OUTPUT_FORMATTED) CC libc/$< 
	@$(CC) -c $< $(CFLAGS) -o $@

libc-builddir:
	@mkdir -p $(BUILD_DIR)/libc

clean:
	rm -rf $(BUILD_DIR)/libc
