#ifndef UTIL_H
#define UTIL_H

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <queue>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#define SPLIT "---------------------\n"
#define SLICE 7 // number of mig slices of an A100
#define PARTITION 5 // number of mig partition type (1/7, 2/7, 3/7, 4/7, 7/7)
typedef int tt;  // timer data time
using namespace std;

enum{
    NOPART,
    STATICPART,
    MYALLOCATE,
    FINAL
};


#endif