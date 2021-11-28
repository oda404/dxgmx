#!/bin/bash

KERNEL_BIN=$1

final=$(mktemp)
tmp=$(mktemp)

$NM -n $KERNEL_BIN > $tmp
printf "1BADADD4\n" > $final
printf "%08X\n" $(wc -l < $tmp) >> $final
cat $tmp >> $final

$OBJCOPY --update-section .ksyms=$final $KERNEL_BIN
cp $final build/$KERNEL_BIN.map

rm -rf $tmp
rm -rf $final
