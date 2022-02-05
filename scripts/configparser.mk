
ifeq ($(shell test '$(DXGMX_CONFIG_OPTIMIZATIONS)' -eq '$(DXGMX_CONFIG_OPTIMIZATIONS)' 2> /dev/null && echo 0 || echo 1),0)
	CFLAGS += -O$(DXGMX_CONFIG_OPTIMIZATIONS)
	CXXFLAGS += -O$(DXGMX_CONFIG_OPTIMIZATIONS)
endif

ifeq ($(DXGMX_CONFIG_DEBUG_SYMS),1)
	CFLAGS += -g
	CXXFLAGS += -g
endif

ifneq ($(DXGMX_CONFIG_LOG_LEVEL),)
	MACROS += -DDXGMX_CONFIG_LOG_LEVEL=$(DXGMX_CONFIG_LOG_LEVEL)
endif

ifeq ($(DXGMX_CONFIG_STACK_PROT),none)
	CFLAGS += -fno-stack-protector
else ifeq ($(DXGMX_CONFIG_STACK_PROT),normal)
	CFLAGS += -fstack-protector
else ifeq ($(DXGMX_CONFIG_STACK_PROT),strong)
	CFLAGS += -fstack-protector-strong
else ifeq ($(DXGMX_CONFIG_STACK_PROT),all)
	CFLAGS += -fstack-protector-all
endif
