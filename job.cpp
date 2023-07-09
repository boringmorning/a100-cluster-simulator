#include "job.h"

Job::Job(int id, int arrivalTime){
    this->id = id;
    this->rt = vector<int>(PARTITION);
    this->arrivalTime = arrivalTime;
}

void Job::run(int gid, vector<int> &slices, tt timer){
    // int pid = sizeToPartition(slices.size());
    int pid = sizeToIndex[slices.size()];
    this->gpuID = gid;
    this->slices = slices;
    this->finishTime = timer + this->rt[pid];
    this->startTime = timer;
}

JobMetrics::JobMetrics(){
    j = nullptr;
    size = 0;
    metrics = 0;
}

JobMetrics::JobMetrics(Job *j, int size){
    this->j = j;
    this->size = size;
    updateMetrics();
}

void JobMetrics::updateMetrics(){
    int idx, newSize;
    switch(this->size){
        case 1:
            newSize = 2;
            break;
        case 2: // tricky part
            newSize = 4;
            break;
        case 4:
            newSize = 7;
            break;
        default:
            printf("wrong size for update metrics: %d\n", this->size);
            exit(1);
    }
    idx = sizeToIndex[newSize];
    metrics = (double)j->rt[idx] * newSize / j->rt[0];
}