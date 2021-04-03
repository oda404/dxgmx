
WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CC             ?= gcc
AR             ?= ar
CFLAGS         := -Wall -Wextra -MMD -MP
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

HEADERS := $(addprefix include/, $(SRC_LIBC))
HEADERS := $(patsubst %.c, %.h, $(HEADERS))

DEPS    := $(addprefix $(BUILD_DIR)/, $(OBJS_LIBC))
DEPS    := $(patsubst %.o, %.d, $(DEPS))

.PHONY: all libc libc-builddir headers clean

all: libc

libc: libc-builddir headers $(addprefix $(BUILD_DIR)/, $(OBJS_LIBC)) Makefile

-include $(DEPS)

$(BUILD_DIR)/libc/%.o : $(patsubst $(BUILD_DIR), ,%.c)
	@$(OUTPUT_FORMATTED) CC libc/$< 
	@$(CC) -c $< $(CFLAGS) -o $@

libc-builddir:
	@mkdir -p $(BUILD_DIR)/libc

headers: $(HEADERS)
	@cp include/* $(SYSROOT_DIR)/usr/include

clean:
	rm -rf $(BUILD_DIR)/libc
