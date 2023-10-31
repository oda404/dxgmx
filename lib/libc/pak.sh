#!/bin/bash tools/pak.sh

setup()
{
    mkdir -p build
    cd build
    cmake .. || exit $?
    cd ..
}

build()
{
    cd build
    make -j$(nproc) || exit $?
    cd ..
}

install()
{
    cd build
    make -j$(nproc) install || exit $?
    cd ..
}

full()
{
    setup
    build
    install
}

clean()
{
    rm -rf build/*
}

mrclean()
{
    clean
    rm -rf build
}
