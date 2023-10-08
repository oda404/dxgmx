
TARGET_NAME = debug

# Extra build flags
EXTRA_CFLAGS += -fstack-protector-all -g -O0

# Modules
include arch/x86/multiboot/module.mk
include drivers/x86/serialsink/module.mk
include drivers/x86/vgasink/module.mk
include drivers/x86/ata/module.mk
include drivers/x86/pit/module.mk

include drivers/builtins/module.mk
include drivers/stackprot/module.mk
include drivers/fat/module.mk
include drivers/ramfs/module.mk
include drivers/fbsink/module.mk
include drivers/pci/module.mk
include drivers/acpi/module.mk
include drivers/devfs/module.mk
