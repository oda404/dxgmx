#!/bin/bash

PACKAGE_NAME="dxgmx_binutils"

BINUTILS_VER="2.37"
TAR_URL="https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VER.tar.gz"
TAR_MD5="1e55743d73c100b7a0d67ffb32398cdb"

BUILD_DIR=$(dirname $0)/build
SRC_DIR=$(dirname $0)/src

while [[ "$#" -gt 0 ]]
do
    case "$1" in
        "--target")
            TARGET="$2"
            shift 2
        ;;
        "--prefix")
            PREFIX="$2"
            shift 2
        ;;
        *)
            shift
        ;;
    esac
done

if [ -z $TARGET ]; then
    echo "No --target specified! Exiting."
    exit 1
fi

if [ -z $PREFIX ]; then
    echo "No --prefix specified! Exiting."
    exit 1
fi

setup() {
    echo Setting up $PACKAGE_NAME...
    mkdir $BUILD_DIR $SRC_DIR 2> /dev/null || true

    export PATH="$PREFIX/bin:$PATH"

	echo "Checking for an existing $SRC_DIR/binutils-$BINUTILS_VER.tar.gz..."
    [ -f $SRC_DIR/binutils-$BINUTILS_VER.tar.gz ] || wget $TAR_URL -P $SRC_DIR

    if ! [ "$TAR_MD5" == "$(md5sum $SRC_DIR/binutils-$BINUTILS_VER.tar.gz | cut -c -32)" ]; then
        echo "Archive checksums don't match! Exiting"
        exit 1
    fi

    echo "Extracting archive..."
    [ -d $SRC_DIR/binutils-$BINUTILS_VER ] || tar -xf $SRC_DIR/binutils-$BINUTILS_VER.tar.gz -C $SRC_DIR
}

build() {
    echo Building $PACKAGE_NAME...

    echo "Configuring build..."
    pushd $BUILD_DIR &> /dev/null

        ../src/binutils-$BINUTILS_VER/configure \
        --target=$TARGET \
        --prefix=$PREFIX \
        --with-sysroot \
        --disable-nls  || exit 1

        make -j$(nproc) || exit 1
        make install -j$(nproc) || exit 1
        
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
