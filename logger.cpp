#include "logger.h"

Logger::Logger(string fileName){
    this->fileName = fileName;
    file.open(fileName, ios::out | ios::trunc);
}

void Logger::finishJob(Job *j){
    this->jobs.push_back(j);
}

void Logger::end(tt timer){
    int avgJCT = 0, avgQD = 0, cnt = 0;
    vector<int> sizeJCT(PARTITION, 0);
    vector<int> partitionCnt(PARTITION, 0);
    for(auto &j: jobs){
        int idx = sizeToIndex[j->slices.size()];
        sizeJCT[idx] += j->finishTime - j->arrivalTime;
        partitionCnt[idx]++;
        avgJCT += j->finishTime - j->arrivalTime;
        avgQD += j->startTime - j->arrivalTime;
        cnt++;
    }
    avgJCT /= cnt;
    avgQD /= cnt;
    file << "total runtime: " << timer << "\n";
    file << "avg JCT: " << avgJCT << "\n";
    file << "avg queueing delay: " << avgQD << "\n";
    file << "partition cnt: " << "\n";
    for(int i=0; i<PARTITION; i++){
        int size = indexToSize[i];
        file << "   " << size << "/7 slices: " << partitionCnt[i] << "\n";
    }
    file << "size JCT: \n";
    for(int i=0; i<PARTITION; i++){
        int size = indexToSize[i];
        if(partitionCnt[i] != 0)
            sizeJCT[i] /= partitionCnt[i];
        file << "   " << size << ": " << sizeJCT[i] << "\n";
    }

    file.close();
}