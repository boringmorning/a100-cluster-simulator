#ifndef A100_H
#define A100_H

#include "util.h"
#include "partition.h"
#include "job.h"

class A100
{
public:
    int id;
    bool empty[SLICE];
    Job *jobTable[SLICE];
    int finishTime[SLICE];
    A100(int id);
    void freePartition(vector<int> &slices);
    void getResource(vector<int> &resource);
    void getPartition(int size, int timer, vector<Partition> &part);
    bool allocate(Job *j, int size, vector<int> &slices);
    void allocatePart(Job *j, Partition &p, vector<int> &slices, tt timer);
    bool hasPartition(int size);
    int freeSliceCnt();
};


#endif