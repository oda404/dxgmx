#!/bin/bash

help() {
	echo "syncroot.sh [options]"
	echo
	echo "Create or update the system root disk image from a source."
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
        "--sysroot")
            SYSROOT="$2"
            shift 2
        ;;
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

SCRIPTDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

${SCRIPTDIR}/mount-root.sh --image-path ${IMG_PATH} --mountpoint ${MOUNTPOINT} --cachefile ${CACHEFILE}

cp -rv ${SYSROOT}/* ${MOUNTPOINT} 

${SCRIPTDIR}/unmount-root.sh --cachefile ${CACHEFILE}
