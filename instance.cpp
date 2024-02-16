#include "instance.h"

Instance::Instance(){
    this->gid = 0;
    this->size = 0;
    this->idx = 0;
}

Instance::Instance(int gid, int size, int idx){
    this->gid = gid;
    this->size = size;
    this->idx = idx;
}