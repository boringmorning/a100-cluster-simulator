#include "partition.h"

Partition::Partition(){
    this->gid = 0;
    this->size = 0;
    this->idx = 0;
}

Partition::Partition(int gid, int size, int idx){
    this->gid = gid;
    this->size = size;
    this->idx = idx;
}