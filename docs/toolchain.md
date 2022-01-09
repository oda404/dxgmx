# The dxgmx toolchain
Right now dxgmx doesn't require an OS specific toolchain and can be built with a generic one. For simplicity's sake some scripts have been created to help with building said toolchain.

## Getting started
You will be building everything from source, so your host machine needs to be ready.
See https://wiki.osdev.org/GCC_Cross-Compiler - "Installing Dependencies" for a list of all the dependencies.

## Building
From the root of the source tree run:
- toolchain/binutils/make.sh --target \<target\> --prefix \<prefix\>
- toolchain/gcc/make.sh --target \<target\> --prefix \<prefix\>

Where \<target\> is the target architecture (for example i686-elf).
