#!/bin/bash

usage() {
    echo "$0 [options]"
    echo ""
    echo "Creates a raw disk image."
    echo ""
    echo "options:"
    echo "  -h,--help         Show this message and exit."
    echo "  -p,--path <path>  Path where to output the image."
    echo "  -s,--size <size>  Image size in the form of <n>(K/M/G)."
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
		"-s"|"--size")
			IMG_SIZE="$2"
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

if [ -z $IMG_SIZE ]; then
	usage
	echo ""
	echo "No -s,--size specified."
	exit 1
fi

qemu-img create -f raw $IMG_PATH $IMG_SIZE
