
# The dxgmx build system
The dxgmx build system uses good old GNU Makefiles, but with a few practices that may be non-standard. 

## make
### When calling `make`, the root Makefile expects the following things:
- [Toolchain](#Toolchain) variables to be exported.
- A [target](#Target) file to be set as an env. variable. 

## Toolchain
 The dxgmx build system uses the following variables during the build:
 - CC:
 - AS:
 - LD: (The actual linker binary).
 - NM:
 - OBJCOPY:
 - LLVM: Set to 1 if a LLVM toolchain is used.
 - DXGMX_ARCH: The target architecture. (For now i686)
 - DXGMX_TOOLCHAIN_ROOT: The root of the toolchain dir. This is where the linker will look for libraries.
 - LIBS: Any extra libraries that should be linked into the kernel. (Usually a runtime library like libgcc.a or libclang_rt.builtins.a is necessary).

 I recommend setting the above variables in a file named **toolchain-\<llvm/gcc>-\<arch>.sh** and then running:
 ```console
 source <script>
 ```
 when building.

## Target
 Targets are used for customizing the kernel. By default the kernel is built with no optimizations, no source level debug information, no extra protection, you get the ideea... The following variables shall be set in a target file: 
 - TARGET_NAME: The name of the target.

 It's now up to you to fuck with any flags you see fit, by appending what you want to variables like:
 - EXTRA_CFLAGS
 - EXTRA_LDFLAGS
 - EXTRA_MACROS
 - EXTRA_WARNINGS
 - LIBS
 - ...

 I'd recommend at least appending **-DDXGMX_CONFIG_LOG_LEVEL=\<log-level>** to EXTRA_MACROS. See [log levels](logging.md##Levels).

 Also note that if you enable optimizations the default -fno-omit-frame-pointer flag will get ignored and stack traces will not work. :)

 You can also include any modules you want in the kernel by using:
 ```console
 include <path/to/module.mk>
 ```

 **Until further notice, the arch/x86/multiboot/module.mk module is mandatory for booting on x86.**

 To use a given target just export it's path with **TARGET_FILE=\<path>**.

I suggest using the following format when naming a target file: **target.\<name>.mk**.

## How do I setup a toolchain ?
### Notes:
- The kernel has been tested with:
    - clang 12.0.1, 13.0.1
- The kernel fails to compile with GCC because clang's -std=c2x is being used and GCC is a bit behind.

 I personally work on the kernel using my host's (arch latest) LLVM, as we don't yet require an OS specific toolchain and the kernel doesn't build with GCC. Nonetheless if you want to use GCC with Binutils, you will need to build them. You can do that by:
- Exporting:
    - TARGET: for now "i686-elf"
    - PREFIX: where the toolchain should be installed **(DO NOT LEAVE THIS BLANK OR /)**.
- Running:
    - toolchain/binutils/make.sh
    - toolchain/gcc/make.sh

These commands will build Binutils & GCC and then install them in PREFIX. You will then need to set DXGMX_TOOLCHAIN_ROOT to whatever you PREFIX was, when actually building the kernel; CC, AS, LD, NM and OBJCOPY should point to your newly built binaries inside your toolchain root bin/.

The exact same goes for building libclang_rt, except you run toolchain/llvm/make.sh instead of binuitls/gcc.
