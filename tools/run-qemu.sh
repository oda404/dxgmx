#!/bin/bash

usage() {
	echo "run-qemu.sh [options]"
	echo
	echo "Run the kernel using QEMU."
	echo 
	echo "options:"
	echo "  -h,--help            Print this message and exit."
	echo "  -e,--emulator <emu>  The emulator to be used. By default qemu."
	echo "  -k,--kernel <path>   Specify the kernel binary path."
	echo "  -i,--iso <path>      Specify the ISO path."
	echo
	echo "Only one of the -i or -k options should be specified at once."
}

if [[ $# -eq 0 ]]; then
	usage
	exit
fi

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"-h"|"--help")
			usage
			exit
		;;
		"-e"|"--emulator")
			EMU="$2"
			shift 2
		;;
		"-k"|"--kernel")
			KERNEL="$2"
			shift 2
		;;
		"-i"|"--iso")
			ISO="$2"
			shift 2
		;;
		*)
			shift
		;;
	esac
done

if [ -z "$KERNEL" ] && [ -z "$ISO" ]; then
    usage
    exit 1
fi


if [ -n "$KERNEL" ] && [ -n "$ISO" ]; then
	echo "-k and -i can't both be specified at the same time."
	exit 1
fi

QEMU=qemu-system-x86_64

QEMU_ARGS=" \
--enable-kvm  \
-cpu host,migratable=off \
-serial stdio \
"

# Display
if [ -n "$DXGMX_QEMU_DISPLAY" ]; then
	QEMU_ARGS+=" -display $DXGMX_QEMU_DISPLAY"
else
	QEMU_ARGS+=" -display none"
fi

# Sysroot disk
if [ -n "$DXGMX_QEMU_SYSROOT_DISK" ]; then
	if [ "$DXGMX_QEMU_USE_IDE" == "y" ]; then
		QEMU_ARGS+=" -drive file=$DXGMX_QEMU_SYSROOT_DISK,if=none,format=raw,id=disk"
		QEMU_ARGS+=" -device ide-hd,drive=disk,bus=ide.0"
	fi
fi

# Memory
if [ -z $DXGMX_QEMU_MEM ]; then
	DXGMX_QEMU_MEM=128M
fi
QEMU_ARGS+=" -m $DXGMX_QEMU_MEM"

# ISO/kernel
if [ -n "$KERNEL" ]; then
	QEMU_ARGS+=" -kernel $KERNEL"
else
	QEMU_ARGS+=" -cdrom $ISO -boot d "
fi

$QEMU $QEMU_ARGS
