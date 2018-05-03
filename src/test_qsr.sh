#!/bin/bash
if [ $# -lt 2 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "<dataset> <outfile>"
        exit 1
fi

make qsr

DATASET=$1
OUTFILE=$2

for (( b=2; b<=32; b*=2 ))
do
    for (( d=2; d<=20; d++ ))
    do
        for (( l=1; l<$d; l++ ))
        do
            echo "Running qsr on $DATASET with blocks $b, depth $d, and lambda $l to $OUTFILE"
            ./qsr -i ../datasets/$DATASET -o $OUTFILE -n $b -d $d -l $l >/dev/null
        done
    done
done

echo "All done"