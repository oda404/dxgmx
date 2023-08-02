#!/bin/bash

help() {
	echo "install-apis.sh [options]"
	echo
	echo "Install the configured kernel APIs."
	echo 
	echo "options:"
	echo "  -h, --help         Print this message and exit."
	echo "  --sysroot          Path to the sysroot."
	echo "  --apis             List of kernel APIs to be exported."
}

while [[ $# -gt 0 ]]
do
	key="$1"
	case "$key" in
		"--sysroot")
			SYSROOT="$2"
			shift 2
		;;
		"--apis")
			APIS="$2"
			shift 2
		;;
		"-h"|"--help")
			help
			exit 1
		;;
		*)
			help
			echo ""
			echo "Unknown argument '$key'"
			shift
			exit 1
		;;
	esac
done

if [ -z $SYSROOT ]; then
	help
	echo ""
	echo "No --sysroot has been specified"
	eixt 1
fi

if [ -z "$APIS" ]; then
	help
	echo ""
	echo "No --apis have been specified"
	eixt 1
fi

mkdir -p $SYSROOT/usr/include/dxgmx

for api in $APIS
do
	src=${api%:*}
	if [[ $api == *":"* ]]; then
		dest=${api#*:*}
	else
		dest=$(basename $api)
	fi

	destpath=$SYSROOT/usr/include/dxgmx/$dest

	mkdir -p $(dirname $destpath)
	cp $src $destpath
done

