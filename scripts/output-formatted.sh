#!/bin/bash

MAX_LENGTH=20
SECOND_LENGTH=`expr "$2" : '.*'`
out_str="  $1    $2"

for (( c=0; c<$MAX_LENGTH-$SECOND_LENGTH; c++ ))
do
    out_str="$out_str "
done

out_str="$out_str$3"

echo "$out_str"
