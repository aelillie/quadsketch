#!/bin/bash
if [ $# -lt 3 ]
    then
        echo "Not enough arguments supplied. Run with:"
        echo "<dataset> <outfile> <#threads>"
        exit 1
fi
#Run as such for MNIST: ./test_qsr.sh mnist mnist_results.csv
make qsr

DATASET=$1
OUTFILE=$2
THREADS=$3

for (( b=2; b<=32; b*=2 ))
do
    for (( d=2; d<=10; d++ ))
    do
        for (( l=1; l<$d; l++ ))
        do
            echo "Running qsr on $DATASET with blocks $b, depth $d, lambda $l, and $THREADS threads to $OUTFILE"
            ./qsr -i ../datasets/$DATASET -o $OUTFILE -n $b -d $d -l $l -t $THREADS >/dev/null
        done
    done
done

echo "All done"