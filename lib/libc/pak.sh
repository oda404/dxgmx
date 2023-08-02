#!/bin/bash

pak_path=$(cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P)
cd $pak_path

cmd=""

usage()
{
    echo "pak.sh [options]"
    echo ""
    echo "Build dxgmx packages. pak assumes that all necessary environment variables are present."
    echo ""
    echo "options:"
    echo "  -h, --help   Print this message and exit"
    echo "  setup        Setup and configure the package"
    echo "  build        Build a configured package package"
    echo "  install      Install a built package"
    echo "  full         setup+build+install. This is the default"
    echo "  clean        Clean build files"
    echo "  mrclean      Clean build and setup files"
}

while [[ $# -gt 0 ]]
do
	arg="$1"

    case "$arg" in
        "setup")
            cmd="setup"
            shift
        ;;

        "build")
            cmd="build"
            shift
        ;;

        "install")
            cmd="install"
            shift
        ;;

        "full")
            cmd="full"
            shift
        ;;

        "clean")
            cmd="clean"
            shift
        ;;

        "mrclean")
            cmd="mrclean"
            shift
        ;;

		"-h"|"--help")
			usage
            shift
			exit
		;;

		*)
            usage
            echo ""
			echo "Unknown argument '$arg'."
            shift
            exit 1
		;;
	esac
done

if [ "$cmd" == "" ]; then
    cmd="full"
fi

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

if [ "$cmd" == "setup" ]; then
    setup
elif [ "$cmd" == "build" ]; then
    build
elif [ "$cmd" == "install" ]; then
    install
elif [ "$cmd" == "full" ]; then
    full
elif [ "$cmd" == "clean" ]; then
    clean
elif [ "$cmd" == "mrclean" ]; then
    mrclean
fi
