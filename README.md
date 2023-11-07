# dxgmx
dxgmx is a scratch written unix-y x86 (i686) kernel made with portability in mind for learning purposes.

## Compiling
### Toolchain
dxgmx uses LLVM as it's toolchain and as such you have two options:
- Using your host LLVM
- Using the dxgmx patched LLVM

The first options is much simpler and faster, but with the host LLVM you will only be able to compile and work on the kernel. If you also want to cross compile a userspace for the kernel you should go with the second option.
Both options are documented [here](building.md#how-do-i-setup-a-toolchain).

### Configuration
dxgmx is configured with it's own configuration utility. Without any modules the kernel will most likely not even boot, or if it does it's going to be useless. If you haven't already you should set `HOSTCC=clang` and `HOSTCXX=clang++` in your env for compiling the configuration utility. GCC will also work for the host toolchain. <br>
Run:
```
$ make conf
```
Play around with anything you like, just note that all `Drivers -> Core` modules are necessary for booting **and** compiling.

### Building
After setting up your toolchain and configuration run:
```console 
$ make
```
If you're doing a clean build you will probably be asked for your sudo password, this is because it's trying to create a system root image, and calls stuff like fdisk, mkfs.fat and losetup. 

## Running in QEMU
You need all the variables in `tools/run-qemu-x86-default-vars.sh` in your environment. <br>
Run:
```console
$ make run
```
This will run the kernel in qemu using the -kernel option.<br>
If you want to run it as an iso, run:
```
$ make iso-run
```
This has the main advantage of being able to use a framebuffer if configured.

## Customizing the system root
When building for the first time a disk image <b>build/image.img</b> will be created. This is a fat32 formatted disk image that the kernel will mount as it's root. If you want to put anything here for the kernel to interact with, you should copy it to the `sysroot/` directory in the dxgmx source root and run:
```
$ make syncroot
```

### libc
The kernel has a very lacking libc used for hacking that you can install into the sysroot by running:
```
$ lib/libc/pak.sh
```
This only works if you are using the dxgmx patched LLVM of course.

## Running on bare metal
First order of business is tweaking the multiboot variables you can find in `drivers/core/multiboot/multiboot.c`. The main thing you probably want to change is the screen resolution. Also the grub configuration needs to have the all_video inserted. <br>
Run:
```console
$ make iso
```
This will create an iso in the root of the source tree with the name of **dxgmx-\<ver\>.iso**.

Copy the iso byte for byte to a bootable medium:
```console
# dd if=path/to/iso of=/dev/sdX status=progress
```
and try to boot it.

The kernel boots (does what it's supposed to do) on my 2019 Lenovo IdeaPad 330S-15ARR.

## Documentation
A growing set of documentation can be found in docs/.