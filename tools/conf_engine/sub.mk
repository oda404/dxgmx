
WD = $(TOOLS_SRCDIR)/conf_engine

TOOL_NCURSES_CONFIG := \
$(BUILDDIR)/$(WD)/ncurses_config

# The config engine depends on jsoncpp for the json parsing (go figure) which kind
# of upsets me a little bit but oh well
$(TOOL_NCURSES_CONFIG): \
$(WD)/ncurses_config.cpp $(WD)/include/dxgmx_libconf/core.h \
$(WD)/core_gen.cpp $(WD)/core_menu.cpp $(WD)/core_ops.cpp $(WD)/core_load.cpp
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX" $<
	@$(HOSTCXX) -I$(TOOLS_SRCDIR)/conf_engine/include \
		$(TOOLS_SRCDIR)/conf_engine/ncurses_config.cpp \
		$(TOOLS_SRCDIR)/conf_engine/core_gen.cpp \
		$(TOOLS_SRCDIR)/conf_engine/core_menu.cpp \
		$(TOOLS_SRCDIR)/conf_engine/core_load.cpp \
		$(TOOLS_SRCDIR)/conf_engine/core_ops.cpp -ljsoncpp -lncursesw -o $@

undefine WD
