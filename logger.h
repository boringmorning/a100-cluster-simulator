#ifndef LOGGER_H
#define LOGGER_H

#include "util.h"
#include "job.h"

extern unordered_map<int,int> sizeToIndex;
extern const int indexToSize[];

class Logger
{
private:
    string fileName;
    fstream file;
    vector<Job*> jobs;
public:
    Logger(string fileName);
    void finishJob(Job *j);
    void end(tt timer);
};


#endif