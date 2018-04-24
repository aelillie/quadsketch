#!/bin/bash
if [ $# -lt 4 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "test.sh <#start> <#end> <#step> <dataset>"
        exit 1
fi

make clean
make

START=$1
END=$2
STEP=$3
DATASET=$4

for (( l=$START; l<=$END; l+=$STEP ))
do
    outfile="$DATASET/grid-results-l$l.out"
    echo "Running grid on $DATASET with $l landmarks to $outfile"
    #./grid -i ../datasets/$DATASET -o $outfile -l $l >/dev/null
done

echo "All done"