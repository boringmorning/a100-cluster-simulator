#include "logger.h"

Logger::Logger(string fileName){
    this->fileName = fileName;
    file.open(fileName, ios::out | ios::trunc);
    util.open(string("util/") + fileName.substr(7), ios::out | ios::trunc);
}

void Logger::finishJob(Job *j){
    this->jobs.push_back(j);
}

void Logger::logUtil(tt timer, double u){
    util << timer << " " << u << "\n"; 
}

void Logger::end(tt timer){
    int cnt = 0;
    double avgQT = 0.0;
    vector<int> sizeJCT(PARTITION, 0);
    vector<int> partitionCnt(PARTITION, 0);
    for(auto &j: jobs){
        avgQT += j->startTime - j->arrivalTime;
        cnt++;
    }
    avgQT /= cnt;
    file << "total runtime: " << timer << "\n";
    file << "avg queueing time: " << fixed << setprecision(2) << avgQT << "\n";
    util << jobs.back()->arrivalTime << "\n";
    util << fixed << setprecision(2) << avgQT << " " << 0 << " " << 1 << "\n";
    util.close();
    file.close();
}