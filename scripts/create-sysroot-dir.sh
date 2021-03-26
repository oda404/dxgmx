#!/bin/bash

SYSROOT_DIR=$1

if ! [ -d "$SYSROOT_DIR" ]; then
    mkdir "$SYSROOT_DIR"
fi

mkdir -p "$SYSROOT_DIR/boot/grub"
mkdir -p "$SYSROOT_DIR/usr/include"
