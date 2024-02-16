#ifndef A100_H
#define A100_H

#include "util.h"
#include "instance.h"
#include "job.h"

class A100
{
public:
    int id;
    bool empty[SLICE];
    Job *jobTable[SLICE];
    int finishTime[SLICE];
    A100(int id);
    void freeSlices(vector<int> &slices);
    void getResource(vector<int> &resource);
    void getGI(int size, int timer, vector<Instance> &GIs);
    void allocateGI(Job *j, Instance &i, vector<int> &slices, tt timer);
};


#endif