
WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include Makefile.config

.PHONY: video clean

video: $(addprefix $(OBJS_DIR)/, $(VIDEO_OBJS))
	@

clean:
	rm -f $(addprefix $(OBJS_DIR)/, $(VIDEO_OBJS))

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	@echo "$(shell $(OUTPUT_FORMATED) CC $(notdir $< $@))"
	@$(CC) -c $< $(CFLAGS) -o $@
