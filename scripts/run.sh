#!/bin/bash

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"-k"|"--kernel-path")
			KERNEL_PATH="$2"
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

if [ -z "$KERNEL_PATH" ]; then
    echo "[iso-run.sh] KERNEL_PATH unset"
    exit 1
elif [ -z "$ARCH" ]; then
    echo "[iso-run.sh] ARCH unset"
    exit 2
fi

qemu-system-x86_64 -m 2G -kernel $KERNEL_PATH
