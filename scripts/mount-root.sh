#!/bin/bash

help() {
	echo "mount-root.sh [options]"
	echo
	echo "Mount the kernel's sysroot on the host machine."
	echo 
	echo "options:"
	echo "  -h, --help         Print this message and exit."
	echo "  --image-path       Path to the sysroot image."
	echo "  --mountpoint       Where on the host machine to mount the image."
	echo "  --cachefile        Cache file used by unmount-root.sh."
}

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"--image-path")
			IMG_PATH="$2"
			shift 2
		;;
		"--mountpoint")
			MOUNTPOINT="$2"
			shift 2
		;;
		"--cachefile")
			CACHEFILE="$2"
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

if [ -z "$IMG_PATH" ] || [ -z "$MOUNTPOINT" ] || [ -z "$CACHEFILE" ]; then
	help
	echo 
	echo "Bad usage"
	exit 1
fi

LOOPDEV=$(losetup --find --show --partscan $IMG_PATH)
mkdir -p $MOUNTPOINT || $(losetup --detach $LOOPDEV && exit 1)
mount ${LOOPDEV}p1 $MOUNTPOINT || $(losetup --detach $LOOPDEV && exit 1)

echo "${LOOPDEV}" > $CACHEFILE

echo Mounted ${LOOPDEV}p1 on $MOUNTPOINT
