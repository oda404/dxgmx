
# The dxgmx build system
The dxgmx build system uses GNU make.

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
 - LLVM: Set to 1 if a LLVM toolchain is used. (Should always be one since we only support LLVM)
 - DXGMX_ARCH: The target architecture. (For now i686)

 NM and OBJCOPY are used for generating and baking the kernel symbols into the kernel image.

 I recommend exporting the above variables in a file, and running the following before compiling:
 ```console
 $ source <toolchainfile>
 ```
 
An example of a toolchain file for i686 can be found in **toolchain/toochain-i686.sh**. Of course this file has my machine's paths.

## Target
Targets are make-like files used for customizing the kernel. By default the kernel is built with no optimizations, no source level debug information, no extra protection, etc. 
 
To use a target file just export it's path with:
```console
$ TARGET_FILE=<path/to/target/file>
```

The following variables need to be set in a target file.
- TARGET_NAME: The name of the target.

It's now up to you to fuck with any flags you see fit, by appending what you want to variables like:
- EXTRA_CFLAGS
- EXTRA_LDFLAGS
- EXTRA_MACROS
- EXTRA_WARNINGS

### Modules:
You can also include any modules you want in the kernel by using:
```makefile
include <path/to/module.mk>
```

### Notes:
- Until further notice, the **arch/x86/multiboot** module is mandatory for booting on x86.
- I'd recommend at least appending **-DDXGMX_CONFIG_LOG_LEVEL=\<log-level>** to EXTRA_MACROS. See [log levels](logging.md##Levels).
- If you enable optimizations the default -fno-omit-frame-pointer flag will get ignored and stack traces will not work. :)

An example of a target file can be found in **toolchain/target.debug.mk**.

## How do I setup a toolchain ?
### Notes:
- The kernel has been tested with:
    - LLVM 12.0.1, 13.0.1 and 15.0.0

### dxgmx specific LLVM:
You will need to build it from source:

- Create a directory on disk that will serve as the toolchain root.
- cd into the dxgmx source tree.
- Run:
```console
$ PREFIX=<path/to/toolchain/dir> toolchain/llvm/make.sh.
```

This is going to take a while depending on your cpu, the script is set to build with -j$(nproc) and compiles clang and lld. Once finished it will install everything in PREFIX.

### Using the host LLVM:
If you don't need the OS-specific toolchain and just want to compile the kernel, you could use your host's LLVM, since clang is a cross compiler by default. 

This method is way faster since you don't have to go through LLVM's balls dropping compile times. If you are going this route, make sure you have clang and lld installed, and just export them in the toolchain file.

### GCC:
Gone, deleted.