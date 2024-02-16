#ifndef INSTANCE_H
#define INSTANCE_H

#include "util.h"

class Instance
{
public:
    vector<int> FT;
    int gid, size, idx, emptyCnt, seg;
    Instance(int gid, int size, int idx);
    Instance();
};

struct compareGI
{
    bool operator()(const Instance &a, const Instance &b)
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
    bool operator()(const Instance &a, const Instance &b)
    {
        return a.seg < b.seg;
    }
};

#endif