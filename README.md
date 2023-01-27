# dxgmx
dxgmx is a scratch written x86 kernel made for learning purposes. Right now the kernel loosely follows Unix conventions, is dependant on GRUB (more specifically the Multiboot 0.6.96 boot specification), and still misses core parts.

# Getting dxgmx up and running

## Toolchain
First order of business is setting up the toolchain. You basically have two options here:
- Using your host LLVM.
- Building the dxgmx patched LLVM.

The first options is much simpler and faster, but with the host LLVM you will only be able to compile the kernel. If you also want to cross compile binaries for the kernel, you should go with the second options.
Both options are documented [here](docs/building.md#how-do-i-setup-a-toolchain).

## Environment & Config
Once you have the toolchain ready, you have two more things to do before compiling.
- Setting the environment variables required for building. More information [here](docs/building.md#toolchain)
- Creating or using an already available target file. More information [here](docs/building.md#target)

Instead of writing these files yourself, you can use the already available ones:<br>

Edit <b>toolchain/toolchain-i686.sh</b> to match your machine's paths, and run:
```console
$ source toolchain/toolchain-i686.sh
```
Next run:
```console
$ export TARGET_FILE=toolchain/target.debug.mk
```

## Compiling
If everything was setup correctly, you can now run:
```console 
$ make
```
and let it rip. If you're doing a clean build, you will probably be asked for your sudo password, this is because it's trying to create a system root image, and calls stuff like fdisk, mkfs.fat and losetup. 

## Running in QEMU
Install QEMU with an x86 BIOS and run:
```console
$ make run
```

## Customizing the system root
When building for the first time a disk image <b>build/image.img</b> will be created. This is a fat32 formatted disk image that the kernel will mount as it's root. If you want to put anything here for the kernel to interact with:<br>

Mount it:
```console
$ make mount-root
```
By default it will get mounted on <b>/mnt/dxgmx-sysroot/</b>.<br>

**Make your changes**

Unmount it:
```console
$ make unmount-root
```

## Bare-metal
Never tried, as it stands it won't boot on anything newer than ~2007.

# Documentation
A growing set of documentation can be found in docs/.