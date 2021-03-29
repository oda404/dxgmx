

WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

.PHONY: arch

OBJS := \
entry.o \
vga_text.o \
purgatory.o 

arch:  $(addprefix $(OBJS_DIR)/, $(OBJS)) Makefile
	@$(OUTPUT_FORMATTED) AR $(notdir $@)
	@$(AR) -cq $(KLIBS_DIR)/arch.a $(addprefix $(OBJS_DIR)/, $(OBJS))
	
$(OBJS_DIR)/%.o : ./**/%.S
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

$(OBJS_DIR)/%.o : ./**/%.c
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

