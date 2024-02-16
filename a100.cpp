#include "a100.h"


A100::A100(int id){
    this->id = id;
    for(int i=0; i<SLICE; i++){
        empty[i] = true;
        finishTime[i] = 0;
    }
}

void A100::freeSlices(vector<int> &slices){
    for(auto &s: slices){
        empty[s] = true;
    }
}

void A100::getResource(vector<int> &resource){
    for(int i=0; i<SLICE; i++){
        if(empty[i]){
            resource[0]++;
        }
    }
    for(int i=0; i+1<SLICE; i+=2){
        if(empty[i] && empty[i+1]){
            resource[1]++;
        }
    }
    if(empty[4] && empty[5] && empty[6]){
        resource[2]++;
    }
    if(empty[0] && empty[1] && empty[2] && empty[3]){
        resource[3]++;
    }
    bool allFree = true;
    for(int i=0; i<SLICE; i++){
        if(!empty[i]){
            allFree = false;
            break;
        }
    }
    if(allFree){
        resource[4]++;
    }
}

void A100::getGI(int size, int timer, vector<Instance> &GIs){
    int currentFT7, currentFT4, currentFT3;
    vector<int> currentFT2(3, timer);
    currentFT7 = currentFT4 = currentFT3 = timer;
    for(int i=0; i<SLICE; i++){
        int idx2 = i / 2;
        if(!empty[i]){
            if(i != SLICE-1){
                currentFT2[idx2] = max(currentFT2[idx2], jobTable[i]->finishTime);
            }
            if(i >= 4){
                currentFT3 = max(currentFT3, jobTable[i]->finishTime);
            }
            else{
                currentFT4 = max(currentFT4, jobTable[i]->finishTime);
            }
            currentFT7 = max(currentFT7, jobTable[i]->finishTime);
        }
    }
    switch(size){
        case 1:
            for(int i=0; i < SLICE; i++){
                if(empty[i]){
                    Instance GI(this->id, 1, i);
                    int idx2 = i / 2;
                    if(i == SLICE-1){
                        GI.FT.push_back(currentFT3);
                    }
                    else{
                        GI.FT.push_back(currentFT2[idx2]);
                    }
                    if(i >= 4){
                        GI.FT.push_back(currentFT3);
                    }
                    else{
                        GI.FT.push_back(currentFT4);
                    }
                    GI.FT.push_back(currentFT7);
                    if(currentFT7 == timer){
                        GI.seg = 7;
                    }
                    else if(GI.FT[1] == timer){
                        if(i >= 4){
                            GI.seg = 3;
                        }
                        else{
                            GI.seg = 4;
                        }
                    }
                    else if(GI.FT[0] == timer){
                        GI.seg = 2;
                    }
                    else{
                        GI.seg = 1;
                    }
                    GIs.push_back(GI);
                }
            }
            break;
        case 2:
            for(int i=0; i<3; i++){
                if(currentFT2[i] == timer){
                    Instance GI(this->id, 2, i*2);
                    if(i*2 >= 4){
                        GI.FT.push_back(currentFT3);
                    }
                    else{
                        GI.FT.push_back(currentFT4);
                    }
                    GI.FT.push_back(currentFT7);
                    if(currentFT7 == timer){
                        GI.seg = 7;
                    }
                    else if(GI.FT[0] == timer){
                        if(i*2 >= 4){
                            GI.seg = 3;
                        }
                        else{
                            GI.seg = 4;
                        }
                    }
                    else{
                        GI.seg = 2;
                    }
                    GIs.push_back(GI);
                }
            }
            break;
        case 3:
            if(currentFT3 == timer){
                Instance GI(this->id, 3, 4);
                GI.FT.push_back(currentFT7);
                if(currentFT7 == timer){
                    GI.seg = 7;
                }
                else{
                    GI.seg = 3;
                }
                GIs.push_back(GI);
            }
            break;
        case 4:
            if(currentFT4 == timer){
                Instance GI(this->id, 4, 0);
                GI.FT.push_back(currentFT7);
                if(currentFT7 == timer){
                    GI.seg = 7;
                }
                else{
                    GI.seg = 4;
                }
                GIs.push_back(GI);
            }
            break;
        case 7:
            if(currentFT7 == timer){
                Instance GI(this->id, 7, 0);
                GI.FT.push_back(timer);
                GI.seg = 7;
                GIs.push_back(GI);
            }
            break;
        default:
            cout << "Wrong partition size!\n";
            cout << size << "\n";
            break;
    }
}

void A100::allocateGI(Job *j, Instance &GI, vector<int> &slices, tt timer){
    for(int s=GI.idx; s<GI.idx + GI.size; s++){
        if(!empty[s]){
            printf("0....0\n");
            exit(1);
        }
        if(timer < finishTime[s]){
            printf("XDD\n");
            exit(1);
        }
        finishTime[s] = j->finishTime;
        slices.push_back(s);
        jobTable[s] = j;
        empty[s] = false;
    }
}