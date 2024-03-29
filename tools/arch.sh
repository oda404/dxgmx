#!/bin/bash

help() {
    echo "Usage $0 [option]
Utility for dxgmx kernel architectures.

Options:
  -h, --help                Show this message and exit.
  --to-srcarch <arch>       Prints the source architecture corresponding to the given one.
  --to-normarch <arch>      Prints the normalized arch corresponding to the given one. Check the root Makefile too see what that means...
  --from-target-trip <trip> Parses the architecture from a target triplet.
"
}

to_srcarch() {
    case $ARCH in
        "x86"|"i686"|"i386"|"x86_64")
            echo "x86"
        ;;
        *)
            echo "undefined"
            exit 1
        ;;
    esac
}

to_normarch() {
    case $ARCH in
        "x86"|"i686"|"i386")
            echo "i686"
        ;;
        "x86_64")
            echo "x86-64"
        ;;
        *)
            echo "undefined"
            exit 1
        ;;
    esac
}

from_target_trip() {
    TRIPLET=$1
    POS=0
    for c in $(seq 1 ${#TRIPLET})
    do
        if [ "${TRIPLET:POS:1}" == "-" ]; then
            break
        fi
        ((POS++))
    done

    if [ $POS -eq 0 ]; then
        echo "undefined"
        exit 1
    fi

    echo ${TRIPLET:0:POS}
}

exit_if_bad_usage() {
    if [[ -z $2 ]]; then
        printf "undefined\n"
        exit 1
    fi
}

if [[ "$#" -eq 0 ]]; then
    exit 1
fi

OPTS=0
while [[ "$#" -gt 0 ]]
do
    case "$1" in
        "--help"|"-h")
            HELP=1
            shift
            ((++OPTS))
        ;;
        "--to-srcarch")
            ARCH="$2"
            exit_if_bad_usage "--to-srcarch" $ARCH
            TO_SRCARCH=1
            shift 2
            ((++OPTS))
        ;;
         "--to-normarch")
            ARCH="$2"
            exit_if_bad_usage "--to-normarch" $ARCH
            TO_NORMARCH=1
            shift 2
            ((++OPTS))
        ;;
        "--from-target-trip")
            TARGET_TRIP="$2"
            exit_if_bad_usage "--from-target-trip" $TARGET_TRIP
            FROM_TARGET_TRIP=1
            shift 2
            ((++OPTS))
        ;;
        *)
            printf "Unrecognized option: $1.\n\n"
            exit 1
        ;;
    esac
done

if [[ $OPTS -gt 1 ]]; then
    echo "$0: Only one option may be specified at once!"
    exit 1
fi

if [[ $HELP -eq 1 ]]; then
    exit
fi

if [[ $TO_SRCARCH -eq 1 ]]; then
    to_srcarch $ARCH
    exit
fi

if [[ $TO_NORMARCH -eq 1 ]]; then
    to_normarch $ARCH
    exit
fi

if [[ $FROM_TARGET_TRIP -eq 1 ]]; then
    from_target_trip $TARGET_TRIP
    exit
fi
