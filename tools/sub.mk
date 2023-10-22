
include tools/libdxgmxconf/sub.mk

TOOL_SYSCALLS_GEN := $(BUILDDIR)/tools/syscalls_generate
$(TOOL_SYSCALLS_GEN): tools/syscalls_generate.cpp
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX LD" $(notdir $@)
	@$(HOSTCXX) $< -ljsoncpp -o $@
