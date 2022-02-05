#!/bin/bash

FIRST_LENGTH=${#1}
TOTAL_LENGTH=7

FIRST_TOKEN=$1
SECOND_TOKEN=$2

if (( $FIRST_LENGTH >= $TOTAL_LENGTH )); then
    echo "scripts/output-formatted.sh: First token: '$1' is too long."
    FIRST_TOKEN="???"
    FIRST_LENGTH=3
fi

OUT_STR="  $FIRST_TOKEN"

for (( c = 0; c < $TOTAL_LENGTH-$FIRST_LENGTH; ++c ))
do
    OUT_STR="$OUT_STR "
done

OUT_STR="$OUT_STR$SECOND_TOKEN"

echo "$OUT_STR"
