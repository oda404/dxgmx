#!/bin/bash

usage() {
	echo "run.sh [options]"
	echo
	echo "Run the kernel in an emulated environment."
	echo 
	echo "options:"
	echo "  -h,--help            Print this message and exit."
	echo "  -e,--emulator <emu>  The emulator to be used. By default qemu."
	echo "  -k,--kernel <path>   Specify the kernel binary path."
	echo "  -i,--iso <path>      Specify the ISO path."
	echo "  -a,--arch <arch>     Specify the architecture"
	echo
	echo "Only one of the -i or -k options should be specified at once."
}

if [[ $# -eq 0 ]]; then
	usage
	exit
fi

EMU=qemu

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
		"-a"|"--arch")
			ARCH="$2"
			shift 2
		;;
		*)
			shift
		;;
	esac
done

if [ -z "$KERNEL" ] && [ -z "$ISO" ] || [ -z "$ARCH" ]; then
    usage
    exit 1
fi

if [ -n "$KERNEL" ] && [ -n "$ISO" ]; then
	echo "-k and -i can't both be specified at the same time."
	exit
fi

case "$EMU" in
	"qemu")
		EMULATOR=qemu-system-
		EMULATOR_OPTS="--enable-kvm -m 2G -cpu host,migratable=off "
		if [ -n "$KERNEL" ]; then
			EMULATOR_OPTS+="-kernel $KERNEL"
		else
			EMULATOR_OPTS+="-cdrom $ISO"
		fi
	;;
	*)
		echo "Invalid emelator $EMU"
		exit
	;;
esac

case "$ARCH" in
	"x86"|"i386"|"i686")
		EMULATOR+=x86_64
	;;
	*)
		echo "Invalid architecure $ARCH"
	;;
esac

$EMULATOR $EMULATOR_OPTS
