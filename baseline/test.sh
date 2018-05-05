#!/bin/bash
if [ $# -lt 5 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "test.sh <#start> <#end> <#step> <dataset> <outfile>"
        exit 1
fi

make clean
make

START=$1
END=$2
STEP=$3
DATASET=$4
OUTFILE=$5

for (( l=$START; l<=$END; l*=$STEP ))
do
    echo "Running grid on $DATASET with $l landmarks to $OUTFILE"
    ./grid -i ../datasets/$DATASET -o $OUTFILE -l $l >/dev/null
done

echo "All done"