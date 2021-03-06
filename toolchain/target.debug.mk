
TARGET_NAME = debug

# Extra build flags
EXTRA_CFLAGS += -fstack-protector-all -g -O0
EXTRA_MACROS += -DDXGMX_CONFIG_LOG_LEVEL=5

# Modules
include kernel/stackprot/module.mk
include fs/fat/module.mk
include fs/ramfs/module.mk
include arch/x86/multiboot/module.mk
include kernel/builtins/module.mk
