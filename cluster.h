#ifndef CLUSTER_H
#define CLUSTER_H

#include "a100.h"
#include "job.h"
#include "logger.h"
#include "instance.h"

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
    priority_queue<Job*, vector<Job*>, compareFT> running_queue;
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
    void bestfit();
    void worstfit();
    vector<Job*> myScheduling();
    vector<Job*> AFCFS_Scheduling();
    void myPlacement(vector<Job*> &plan);
    void bestfitPlacement(vector<Job*> &plan);
    void worstfitPlacement(vector<Job*> &plan);
};


#endif