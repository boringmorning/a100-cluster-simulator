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
#include <cmath>
#define SPLIT "---------------------\n"
#define SLICE 8 // number of mig slices of an A100
#define PARTITION 4 // number of mig partition type (1/8, 2/8, 4/8, 8/8)
typedef int tt;  // timer data time
using namespace std;

enum{
    MYALGO,
    SIMPLE,
    BESTFIT,
    WORSTFIT
};


#endif