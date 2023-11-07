# The dxgmx build system
The dxgmx build system uses GNU make.

## Toolchain
 The dxgmx build system uses the following evnironment variables during the build:
 - CC:
 - AS:
 - LD: (The actual linker binary).
 - NM:
 - OBJCOPY:
 
An example of a toolchain file for i686 can be found in **toolchain/toolchain-i686.sh**. Of course this file has my machine's paths.

## How do I setup a toolchain ?

### dxgmx specific LLVM:
If you want to compile a userspace for the kernel (if you want to do anything interesting) you'll need this:

- Create a directory on disk that will serve as the toolchain root.
- cd into the dxgmx source tree.
- Run:
```console
$ PREFIX=<path/to/toolchain/dir> toolchain/llvm/make.sh.
```

This is going to take a while depending on your cpu, the script is set to build with -j$(nproc) and compiles clang and lld. Once finished it will install everything in PREFIX.<br>
You then need to export the above variables to the freshly installed llvm binaries.

### Using the host LLVM:
If you don't need the OS-specific toolchain and just want to compile the kernel, you could use your host's LLVM, since clang is a cross compiler by default.

This method is way faster since you don't have to go through LLVM's balls dropping compile times. If you're going this route just export the above variables to corresponding the llvm binaries

### GCC:
Gone, deleted.

### Notes:
- The current version of the kernel has been tested with:
    - LLVM 15.0.0