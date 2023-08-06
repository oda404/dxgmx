#/usr/bin/bash

[ $($CC --version | awk 'NR==1 { print $1 }') == "clang" ] && echo 1 || echo 0
