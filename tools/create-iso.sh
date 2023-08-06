#!/bin/bash

help() {
	echo "create-iso.sh [options]"
	echo
	echo "Create a bootable dxgmx iso image with GRUB."
	echo 
	echo "options:"
	echo "  -h, --help         Print this message and exit."
	echo "  --sysroot          Path to the sysroot."
	echo "  --kernel           Path to kernel binary."
	echo "  --out              Destination output path."
}

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"--sysroot")
			SYSROOT="$2"
			shift 2
		;;
		"--kernel")
			KERNEL="$2"
			shift 2
		;;
		"--out")
			OUT="$2"
			shift 2
		;;
		"-h"|"--help")
			help
			exit 1
		;;
		*)
			shift
		;;
	esac
done

if [ -z "$SYSROOT" ] || [ -z "$KERNEL" ] || [ -z "$OUT" ]; then
    help
    echo "Wrong usage"
    exit 1
fi

mkdir -p $SYSROOT/boot/grub

cp $KERNEL $SYSROOT/boot/

echo "timeout=0"                           > $SYSROOT/boot/grub/grub.cfg
echo "menuentry \"$KERNEL\" {"            >> $SYSROOT/boot/grub/grub.cfg
echo "	insmod all_video"                 >> $SYSROOT/boot/grub/grub.cfg
echo "	multiboot /boot/$KERNEL"          >> $SYSROOT/boot/grub/grub.cfg
echo "}"                                  >> $SYSROOT/boot/grub/grub.cfg
grub-mkrescue -o $OUT $SYSROOT
