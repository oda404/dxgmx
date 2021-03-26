

WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include Makefile.config

.PHONY: init clean

init: $(addprefix $(OBJS_DIR)/, $(INIT_OBJS)) Makefile
	@

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f $(addprefix $(OBJS_DIR)/, $(INIT_OBJS))
