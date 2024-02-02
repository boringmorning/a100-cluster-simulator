#include "a100.h"


A100::A100(int id){
    this->id = id;
    for(int i=0; i<SLICE; i++){
        empty[i] = true;
        finishTime[i] = 0;
    }
}

void A100::freePartition(vector<int> &slices){
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

void A100::getPartition(int size, int timer, vector<Partition> &part){
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
                    Partition p(this->id, 1, i);
                    int idx2 = i / 2, idx4 = i / 4;
                    p.FT.push_back(currentFT2[idx2]);
                    p.FT.push_back(currentFT4[idx4]);
                    p.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        p.seg = 8;
                    }
                    else if(currentFT4[idx4] == timer){
                        p.seg = 4;
                    }
                    else if(currentFT2[idx2] == timer){
                        p.seg = 2;
                    }
                    else{
                        p.seg = 1;
                    }
                    part.push_back(p);
                }
            }
            break;
        case 2:
            for(int i=0; i<4; i++){
                if(currentFT2[i] == timer){
                    Partition p(this->id, 2, i*2);
                    int idx4 = i * 2 / 4;
                    p.FT.push_back(currentFT4[idx4]);
                    p.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        p.seg = 8;
                    }
                    else if(currentFT4[idx4] == timer){
                        p.seg = 4;
                    }
                    else{
                        p.seg = 2;
                    }
                    part.push_back(p);
                }
            }
            break;
        case 4:
            for(int i=0; i<2; i++){
                if(currentFT4[i] == timer){
                    Partition p(this->id, 4, i*4);
                    p.FT.push_back(currentFT8);
                    if(currentFT8 == timer){
                        p.seg = 8;
                    }
                    else{
                        p.seg = 4;
                    }
                    part.push_back(p);
                }
            }
            break;
        case 8:
            if(currentFT8 == timer){
                Partition p(this->id, 8, 0);
                p.FT.push_back(timer);
                p.seg = 8;
                part.push_back(p);
            }
            break;
        default:
            cout << "Wrong partition size!\n";
            cout << size << "\n";
            break;
    }
}

void A100::allocatePart(Job *j, Partition &p, vector<int> &slices, tt timer){
    for(int s=p.idx; s<p.idx + p.size; s++){
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