#!/bin/bash

NGPU=20
NCONFIG=5
NCASE=5
NALGO=3

# generate workload
python genJob.py $NCONFIG $NCASE

#simulated experiments
for (( i=0; i<$NCONFIG; i++ )) # exp config
do
    for (( j=0; j<$NCASE; j++ )) # testcase
    do
        for (( k=0; k<$NALGO; k++ )) # algo
        do
            ./main.exe $NGPU $i $j $k
        done
    done
done

# analyze result
python result.py $NCONFIG $NCASE