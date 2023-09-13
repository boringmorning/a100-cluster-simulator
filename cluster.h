#ifndef CLUSTER_H
#define CLUSTER_H

#include "a100.h"
#include "job.h"
#include "logger.h"
#include "partition.h"

class Cluster
{
private:
    int ngpu, algo, epoch;
    double util;
    bool heavy;
    tt timer;
    Logger *logger;
    vector<A100> gpus;
    vector<Job*> jobs;
    vector<int> resource;
    vector<int> pcnt;
    queue<Job*> job_queue;
    priority_queue<Job*, vector<Job*>, compareFinish> running_queue;
    vector<priority_queue<Job*, vector<Job*>, compareArrival>> readyJobs;
public:
    Cluster();
    Cluster(int ngpu, Logger *logger, int algo);
    void run();
    void newJob(Job *j);
    void updateStatus();
    void finishJob(Job *j);
    void schedule();
    void myAlgo();
    void final();
    void best();
    bool validScaleUp(int size);
    vector<vector<Job*>> myAllocate();
    void placement(vector<vector<Job*>> &plan);
    void myPlacement(vector<vector<Job*>> &plan);
};


#endif