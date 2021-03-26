#!/bin/bash

ARCH="$1"

if [ -z "$ARCH" ]; then
    echo "[arch-to-srcarch.sh] ARCH unset"
    exit 1
fi

case "$ARCH" in
    "x86"|"i368"|"i686")
        echo "x86"
    ;;
    *)
        echo "undefined"
    ;;
esac
