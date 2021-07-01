#!/bin/bash

TRIPLET="$1"

if [ -z $TRIPLET ]; then
    echo "undefined"
    exit 1
fi

pos=0
for c in $(seq 1 ${#TRIPLET})
do
    if [ "${TRIPLET:pos:1}" == "-" ]; then
        break
    fi
    ((pos++))
done

if [ $pos -eq 0 ]; then
    echo "undefined"
    exit 2
fi

echo ${TRIPLET:0:pos}
