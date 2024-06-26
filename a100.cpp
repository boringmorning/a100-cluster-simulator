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
    for(int i=0; i+3<SLICE; i+=4){
        if(empty[i] && empty[i+1] && empty[i+2] && empty[i+3]){
            resource[2]++;
        }
    }
    bool allFree = true;
    for(int i=0; i<SLICE; i++){
        if(!empty[i]){
            allFree = false;
            break;
        }
    }
    if(allFree){
        resource[3]++;
    }
}

void A100::getGI(int size, int timer, vector<Instance> &GIs){
    int currentFT8 = timer;
    vector<int> currentFT2(4, timer), currentFT4(2, timer);
    for(int i=0; i<SLICE; i++){
        int idx2 = i / 2;
        int idx4 = i / 4;
        if(!empty[i]){
            currentFT2[idx2] = max(currentFT2[idx2], jobTable[i]->finishTime);
            currentFT4[idx4] = max(currentFT4[idx4], jobTable[i]->finishTime);
            currentFT8 = max(currentFT8, jobTable[i]->finishTime);
        }
    }
    switch(size){
        case 1:
            for(int i=0; i < SLICE; i++){
                if(empty[i]){
                    Instance GI(this->id, 1, i);
                    int idx2 = i / 2, idx4 = i / 4;
                    GI.FT.push_back(currentFT2[idx2]);
                    GI.FT.push_back(currentFT4[idx4]);
                    GI.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        GI.seg = 8;
                    }
                    else if(currentFT4[idx4] == timer){
                        GI.seg = 4;
                    }
                    else if(currentFT2[idx2] == timer){
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
            for(int i=0; i<4; i++){
                if(currentFT2[i] == timer){
                    Instance GI(this->id, 2, i*2);
                    int idx4 = i * 2 / 4;
                    GI.FT.push_back(currentFT4[idx4]);
                    GI.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        GI.seg = 8;
                    }
                    else if(currentFT4[idx4] == timer){
                        GI.seg = 4;
                    }
                    else{
                        GI.seg = 2;
                    }
                    GIs.push_back(GI);
                }
            }
            break;
        case 4:
            for(int i=0; i<2; i++){
                if(currentFT4[i] == timer){
                    Instance GI(this->id, 4, i*4);
                    GI.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        GI.seg = 8;
                    }
                    else{
                        GI.seg = 4;
                    }
                    GIs.push_back(GI);
                }
            }
            break;
        case 8:
            if(currentFT8 == timer){
                Instance GI(this->id, 8, 0);
                GI.FT.push_back(timer);
                GI.seg = 8;
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