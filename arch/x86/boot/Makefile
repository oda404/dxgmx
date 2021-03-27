

WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include Makefile.config

.PHONY: boot clean

boot: $(addprefix $(OBJS_DIR)/, $(ARCH_BOOT_OBJS))
	@

clean:
	rm -f $(addprefix $(OBJS_DIR)/, $(ARCH_BOOT_OBJS))

$(OBJS_DIR)/%.o : $(WD_PATH)/%.S
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@
