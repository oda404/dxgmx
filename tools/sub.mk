
include $(TOOLS_SRCDIR)/conf_engine/sub.mk

TOOL_SYSCALLS_GEN := $(BUILDDIR)/$(TOOLS_SRCDIR)/syscalls_generate
$(TOOL_SYSCALLS_GEN): $(TOOLS_SRCDIR)/syscalls_generate.cpp
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX" $<
	@$(HOSTCXX) $< -o $@
