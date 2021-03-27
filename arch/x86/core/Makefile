
WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include Makefile.config

.PHONY: core clean

core: $(addprefix $(OBJS_DIR)/, $(ARCH_CORE_OBJS))
	@

clean:
	rm -f $(addprefix $(OBJS_DIR)/, $(ARCH_CORE_OBJS))

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@
