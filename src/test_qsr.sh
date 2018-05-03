#!/bin/bash
if [ $# -lt 2 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "test_qsr.sh <start> <end> <step> <#blocks> <dataset> <outfile>"
        exit 1
fi

make qsr

START=$1
END=$2
STEP=$3
BLOCKS=$4
DATASET=$5
OUTFILE=$6

for (( k=$START; k<=$END; k++ ))
do
    LAMBDA=$k
    DEPTH=$((k+$STEP))
    echo "Running qsr on $DATASET with blocks $BLOCKS, depth $DEPTH, and lambda $LAMBDA to $outfile"
    ./qsr -i ../datasets/$DATASET -o $OUTFILE -n $BLOCKS -d $DEPTH -l $LAMBDA >/dev/null
done

echo "All done"
#For example: qs -i ../datasets/sift -o sift_results.txt -n 8 -d 7 -l 5