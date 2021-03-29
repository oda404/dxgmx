

WD_PATH := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

SRC  := \
main.c 

OBJS := \
main.o

.PHONY: init clean

init: $(addprefix $(OBJS_DIR)/, $(OBJS)) Makefile
	@$(OUTPUT_FORMATTED) AR $(notdir $@)
	@$(AR) -cq $(KLIBS_DIR)/init.a $(addprefix $(OBJS_DIR)/, $(OBJS))

$(OBJS_DIR)/%.o : $(WD_PATH)/%.c
	@$(OUTPUT_FORMATTED) CC $(notdir $< $@)
	@$(CC) -c $< $(CFLAGS) -o $@

clean:
	rm -f $(addprefix $(OBJS_DIR)/, $(OBJS))
