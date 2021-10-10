# dxgmx configuration options
Below is a list of all the configuration options that can be set in a [*.buildtarget](build-system.md##*.buildtarget) in the format of **[target-name]=[value]**:
- **CONFIG_STACK_PROT**: (none,normal,strong,all) Can be set to one of the previous options. Each of which correspond to GCC's -fstack-protector-*.
- **CONFIG_BOOTSPEC**: (multiboot) Can be set to one or more of the previous options. Tells the kernel that it's going to have available the specified boot specifications. 
- **CONFIG_LOGLVL**: Can be set to any [log level](logging.md##Levels).
- **CONFIG_OPTIMIZATIONS**: (0,1,2,3) Can be set to one of the previous options. Each of which correspond to GCC's -O* flag.
- **CONFIG_DEBUG_SYMS**: Set to 1 if source level debug symbols should be retained in the binary, 0 otherwise.