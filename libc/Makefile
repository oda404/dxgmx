
WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CC     ?= gcc
# libc will not be compiled with the same cflags 
# and defs as the rest of the kernel
CFLAGS := -Wall -Wextra
DEFS   := 

ifeq ($(LIBC_FREESTANDING), 1)
	CFLAGS += -ffreestanding --sysroot=$(SYSROOT_DIR) -isystem=/usr/include/
	DEFS   += -D__LIBC_FREESTANDING__
else
	
endif

CFLAGS += $(DEFS)

SRC := \
stdlib.c \
string.c

OBJS := \
stdlib.o \
string.o

HEADERS := \
include/stdlib.h \
include/string.h

.PHONY: libc libc-headers

libc: libc-headers $(addprefix $(OBJS_DIR)/, $(OBJS)) Makefile
	@$(OUTPUT_FORMATTED) AR $(notdir $@)
	@$(AR) -cq $(KLIBS_DIR)/libc.a $(addprefix $(OBJS_DIR)/, $(OBJS))
	

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

libc-headers: $(HEADERS)
	@mkdir -p $(SYSROOT_DIR)/usr/include
	@cp include/* $(SYSROOT_DIR)/usr/include
