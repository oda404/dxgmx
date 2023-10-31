#!/bin/bash

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

PAK_SCRIPT=$(basename "$1")
PAK_ROOT=$(cd "$(dirname "$1")" ; pwd -P)
shift 
cmd="full"

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

cd $PAK_ROOT
source $PAK_SCRIPT
$cmd
