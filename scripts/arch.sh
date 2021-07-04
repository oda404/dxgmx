#!/bin/bash

HELP_MSG="Usage arch.sh [options]
Utility for dxgmx kernel architectures.

Options:
  -h, --help           Show this message and exit
  --valid-srcarchs     Prints all valid srch architectures.
  --to-srcarch <arch>  Prints the src architecture corresponding to the given one.
"

SRCARCHS=(
    x86 
)

if [[ "$#" -eq 0 ]]; then
    echo -e "$HELP_MSG"
    exit 1
fi

while [[ "$#" -gt 0 ]]
do
    case "$1" in
        "--help"|"-h")
            shift
            echo -e "$HELP_MSG"
            exit
        ;;
        "--valid-srcarchs")
            echo "${SRCARCHS[*]}"
            exit
        ;;
        "--to-srcarch")
            ARCH="$2"
            shift 2
        ;;
        *)
            shift
        ;;
    esac
done

case $ARCH in
    "x86"|"i686"|"i386")
        echo "x86"
    ;;
    *)
        echo "undefined"
    ;;
esac
