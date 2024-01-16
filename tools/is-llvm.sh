#/usr/bin/bash

if [ "$CC" == "" ]; then
    echo 0
    exit 0
fi

[ $($CC --version | awk 'NR==1 { print $1 }') == "clang" ] && echo 1 || echo 0
