#!/bin/bash

FIRST_LENGTH=${#1}
TOTAL_LENGTH=11

FIRST_TOKEN=$1
# Strip unneeded '/'
SECOND_TOKEN=$(echo $2 | sed s#//*#/#g)

if (( $FIRST_LENGTH >= $TOTAL_LENGTH )); then
    echo "$0: First token: '$1' is too long."
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
