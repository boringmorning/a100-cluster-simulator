#ifndef PARTITION_H
#define PARTITION_H

#include "util.h"

class Partition
{
public:
    vector<int> FT;
    int gid, size, idx, emptyCnt, seg;
    Partition(int gid, int size, int idx);
    Partition();
};

struct comparePartition
{
    bool operator()(const Partition &a, const Partition &b)
    {
        int n = a.FT.size();
        for(int i=0; i<n; i++){
            if(a.FT[i] == b.FT[i]){
                continue;
            }
            return a.FT[i] < b.FT[i];
        }
        return true;
    }
};

struct compareSeg
{
    bool operator()(const Partition &a, const Partition &b)
    {
        return a.seg < b.seg;
    }
};

#endif