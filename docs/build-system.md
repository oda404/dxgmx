
# The dxgmx build system
The dxgmx build system uses good old Makefiles, but with a few practices that may be non-standard. 

## make
### When calling `make`, the root Makefile expects the following files to be present in the root directory and/or explicitly set as env. variables:
- [buildconfig](##buildconfig): Defines for which platform the kernel is built.
- [*.buildtarget](##*.buildtarget): Defines how the kernel is built.

## buildconfig
### A makefile-like file that defines variables about the build environment and target architecture. The file can either be automatically picked up if placed in the root of the source tree, or it's path can be explictly set as an env. variable using **BUILDCONFIG=<...>**. **If no buildconfig file is found the build will fail early on.** Below is list of all the relevant options that **must** be set in a buildconfig file.
- **HAS_BUILDCONFIG**: Must be set to 1.
- **CC**: The C compiler binary path.
- **CXX**: The C++ compiler binary path(currently unused).
- **AS**: The assembler binary path.
- **LD**: The linker binary path.
- **TARGET_TRIPLET**: The target triplet.
- **IS_CROSS_COMP**: 1 if cross-compiling, 0 if not.
- **BUILDTARGET**: [optional] A [*.buildtarget](##*.buildtarget) file.

## *.buildtarget
### A makefile-like file defining build configuration options. A *.buildtarget's file path can be set using **BUILDTARGET=<...>** either in the [buildconfig](##buildconfig) file or as an env. variable. If no *.buildtarget file is specifed the kernel *will* build but the output binary will probably have no output as no log level has been set, no optimizations and no extra security. Below is a list of all the relevant options that **must** be set in a *.buildtarget file:
- **HAS_BUILDTARGET**: Must be set to 1.
- **BT_NAME**: The name of the target.
- **BT_CFLAGS**: Extra flags for the C compiler.
- **BT_CXXFLAGS**: Extra flags for the C++ compiler.
- **BT_LDFLAGS**: Extra flags for the linker.
- **BT_MACROS**: Extra macros for the C/C++ pre-processor.
- **BT_WARNINGS**: Extra warnings for the C/C++ compiler.
- **Config options**: [optional] A list of configuration options. See config-options.md

## Notes
- For now the kernel can only be built with GCC and GNU binutils.