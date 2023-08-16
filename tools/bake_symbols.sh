#!/bin/bash

KERNEL_BIN=$1

tmp=$(mktemp)
final=build/$KERNEL_BIN.map

$NM -n $KERNEL_BIN | awk '{ if($2 == "T" || $2 == "t") printf("%s %s %s\n", $1, $2, $3); }' > $tmp

printf "1BADADD4\n" > $final
printf "%08X\n" $(wc -l < $tmp) >> $final
cat $tmp >> $final

$OBJCOPY --update-section .ksyms=$final $KERNEL_BIN

rm -rf $tmp
