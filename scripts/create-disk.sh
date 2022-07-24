#!/bin/bash

usage() {
    echo "$0 [options]"
    echo ""
    echo "Creates a raw disk image."
    echo ""
    echo "options:"
    echo "  -h,--help         Show this message and exit."
    echo "  -p,--path <path>  Path where to output the image."
}

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"-h"|"--help")
			usage
			exit
		;;
		"-p"|"--path")
			IMG_PATH="$2"
			shift 2
		;;
		*)
			shift
		;;
	esac
done

if [ -z $IMG_PATH ]; then
	usage
	echo ""
	echo "No -p,--path specified."
	exit 1
fi

IMG_SIZE=256M

dd if=/dev/zero of=$IMG_PATH bs=$IMG_SIZE count=1

LOOPDEV=$(sudo losetup --find --show --partscan $IMG_PATH)

echo "label: dos" | sudo sfdisk $LOOPDEV
echo "${LOOPDEV}p1 : start= 1, size= 522240, type=83" | sudo sfdisk $LOOPDEV
sudo mkfs.fat -F32 ${LOOPDEV}p1

sudo losetup --detach $LOOPDEV
