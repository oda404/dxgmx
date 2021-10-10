# dxgmx
dxgmx is a scratch written x86 kernel made for learning purposes. Right now the kernel loosely follows Unix conventions, is dependant on GRUB (more specifically the Multiboot 0.6.96 boot specification), and still misses core parts.

## Getting dxgmx up and running
dxgmx has only been tested using a generic i686-elf **GCC** compiler and **GNU Binutils**, but I don't see why any generic x86-elf **GCC** compiler and **GNU Binutils** would fail.<br>
Once you have the **GCC** compiler and **GNU Binuitls** ready, continue by reading [The dxgmx build system](docs/build-system.md).

## Documentation
A growing set of documentation can be found in docs/.