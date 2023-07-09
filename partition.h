#ifndef PARTITION_H
#define PARTITION_H

#include "util.h"

class Partition
{
public:
    int gid, size, idx, FT;
    Partition(int gid, int size, int idx);
    Partition();
};

struct comparePartition
{
    bool operator()(const Partition &a, const Partition &b)
    {
        return a.FT < b.FT;
    }
};

#endif