#ifndef JOB_H
#define JOB_H

#include "util.h"

extern unordered_map<int,int> sizeToIndex;

class Job
{
public:
    int id, gpuID, speed, limitSize;
    tt arrivalTime, startTime, finishTime;
    vector<int> rt; // job runtime for each mig partition size
    vector<int> slices;
    Job(int id, int arrivalTime);
    void run(int gid, vector<int> &slices, tt timer);
};

class JobMetrics{
public:
    Job *j;
    int size;
    double metrics;
    JobMetrics();
    JobMetrics(Job *j, int size);
    ~JobMetrics(){};
    void updateMetrics();
};

struct compareMetrics{
    bool operator()(const JobMetrics &a, const JobMetrics &b)
    {
        return a.metrics > b.metrics;
    }
};

struct compareFinish
{
    bool operator()(const Job *a, const Job *b)
    {
        return a->finishTime > b->finishTime;
    }
};

struct compareFinish2
{
    bool operator()(const Job *a, const Job *b)
    {
        return a->finishTime < b->finishTime;
    }
};

struct compareSpeed
{
    bool operator()(const Job *a, const Job *b)
    {
        return a->speed > b->speed;
    }
};

struct compareArrival
{
    bool operator()(const Job *a, const Job *b)
    {
        return a->arrivalTime > b->arrivalTime;
    }
};

#endif