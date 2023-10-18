
WD = tools/libdxgmxconf

# common libconf
LIB_CONF     := $(BUILDDIR)/$(WD)/libdxgmxconf.a
LIB_CONF_OBJS := $(BUILDDIR)/$(WD)/core.cpp.o $(BUILDDIR)/$(WD)/core_tree.cpp.o

$(LIB_CONF): $(LIB_CONF_OBJS)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "AR" $(notdir $@)
	@$(AR) rcs $(LIB_CONF) $(LIB_CONF_OBJS)

# ncurses conf frontend
# The config engine depends on jsoncpp for the json parsing (go figure) which kind
# of upsets me a little bit but oh well
TOOL_NCURSES_CONFIG       := $(BUILDDIR)/$(WD)/ncurses_config
TOOL_NCURSES_CONFIG_OBJS  := $(BUILDDIR)/$(WD)/ncurses_config.cpp.o

$(TOOL_NCURSES_CONFIG): $(TOOL_NCURSES_CONFIG_OBJS) $(LIB_CONF)
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX LD" $(notdir $@)
	@$(HOSTCXX) $(TOOL_NCURSES_CONFIG_OBJS) $(LIB_CONF) -ljsoncpp -lncursesw -o $@

# kconfig header generator
TOOL_KCONFIG_H_GENERATOR := $(BUILDDIR)/$(WD)/kconfig_h_generator

$(TOOL_KCONFIG_H_GENERATOR): $(WD)/kconfig_h_generator.cpp
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX LD" $(notdir $@)
	@$(HOSTCXX) $< -o $@

$(BUILDDIR)/$(WD)/%.cpp.o: $(WD)/%.cpp $(WD)/include/dxgmx_libconf/core.h
	@mkdir -p $(dir $@)
	@$(PRETTY_PRINT) "HOSTCXX" $<
	@$(HOSTCXX) -c -Itools/libdxgmxconf/include $< -o $@

undefine WD
