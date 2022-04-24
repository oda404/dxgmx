#!/bin/bash

PACKAGE_NAME="dxgmx_gcc"

GCC_VER="11.2.0"
TAR_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VER/gcc-$GCC_VER.tar.gz"
TAR_MD5="dc6886bd44bb49e2d3d662aed9729278"

BUILD_DIR=$(dirname $0)/build
SRC_DIR=$(dirname $0)/src

setup() {
    echo Setting up $PACKAGE_NAME...
    mkdir $BUILD_DIR $SRC_DIR 2> /dev/null || true

    echo "Prefix is \"$PREFIX\""

    export PATH="$PREFIX/bin:$PATH"

	echo "Checking for an existing $SRC_DIR/gcc-$GCC_VER.tar.gz..."
    [ -f $SRC_DIR/gcc-$GCC_VER.tar.gz ] || wget $TAR_URL -P $SRC_DIR

    if ! [ $TAR_MD5 == $(md5sum $SRC_DIR/gcc-$GCC_VER.tar.gz | cut -c -32) ]; then
        echo "Archive checksums don't match! Exiting"
        exit 1
    fi

    echo "Extracting archive..."
    [ -d $SRC_DIR/gcc-$GCC_VER ] || tar -xf $SRC_DIR/gcc-$GCC_VER.tar.gz -C $SRC_DIR
}

build() {
    echo Building $PACKAGE_NAME...

    echo "Configuring build..."
    pushd $BUILD_DIR &> /dev/null

        ../src/gcc-$GCC_VER/configure \
        --target=$TARGET \
        --prefix=$PREFIX \
        --enable-languages=c \
        --disable-nls \
        --without-headers || exit 1

        make all-gcc -j$(nproc) || exit 1
        make all-target-libgcc -j$(nproc) || exit 1
        make install-gcc -j$(nproc) || exit 1
        make install-target-libgcc -j$(nproc) || exit 1
        
    popd &> /dev/null
}

clean() {
    rm -rf $SRC_DIR
}

mrclean() {
    clean
    rm -rf $BUILD_DIR
}

setup
build
