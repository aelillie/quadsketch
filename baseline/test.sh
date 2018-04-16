#!/bin/bash
if [ $# -lt 2 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "test.sh <#landmarks> <dataset>"
        exit 1
fi

make #build program

STEP=$1
START=10
END=170

DATASET=$2

for (( l=$START; l<=$END; l+=$STEP ))
do
    outfile="grid-$DATASET-results-l$l.out"
    echo "Running grid on $DATASET with $l landmarks to $outfile"
    ./grid -i ../datasets/$DATASET -o $outfile -l $l >/dev/null
done

echo "All done"