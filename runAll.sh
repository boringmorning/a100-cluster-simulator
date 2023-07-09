#!/bin/bash

NGPU="10"

for i in {0..4} # workload
do
    for j in {2..3} # algo
    do
        ./main $NGPU $i $j
    done
done