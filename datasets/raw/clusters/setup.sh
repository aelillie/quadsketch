#!/bin/bash
echo "Cleaning up..."
make clean >/dev/null
./clean_datasets.sh
echo "Building..."
make
echo "Running Clusters generator..."
./gen -n 1000000 -d 128 -i 100 -a 600 -q 10000