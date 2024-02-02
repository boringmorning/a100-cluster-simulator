#ifndef CLUSTER_H
#define CLUSTER_H

#include "a100.h"
#include "job.h"
#include "logger.h"
#include "partition.h"

class Cluster
{
private:
    int ngpu, algo;
    double util;
    bool heavy;
    tt timer;
    Logger *logger;
    vector<A100> gpus;
    vector<Job*> jobs;
    vector<int> resource;
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
    void mySimple();
    void bestfit();
    void worstfit();
    vector<vector<Job*>> myAllocate();
    void myPlacement(vector<vector<Job*>> &plan);
    void simplePlacement(vector<vector<Job*>> &plan);
    void bestfitPlacement(vector<vector<Job*>> &plan);
    void worstfitPlacement(vector<vector<Job*>> &plan);
};


#endif