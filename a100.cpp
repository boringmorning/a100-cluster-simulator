#include "a100.h"


A100::A100(int id){
    this->id = id;
    for(int i=0; i<SLICE; i++){
        empty[i] = true;
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
    if(empty[4] && empty[5] && empty[6]){
        resource[2]++;
    }
    if(empty[0] && empty[1] && empty[2] && empty[3]){
        resource[3]++;
    }
    bool allFree = true;
    for(int i=0; i<7; i++){
        if(!empty[i]){
            allFree = false;
            break;
        }
    }
    if(allFree){
        resource[4]++;
    }
}

void A100::getPartition(int size, int timer, vector<Partition> &part){
    int currentFT3 = 0, currentFT4 = 0, currentFT7 = 0;
    int sum = 0, cnt = 0;
    vector<int> currentFT2(3, 0);
    for(int i=0; i<SLICE; i++){
        if(!empty[i]){
            currentFT7 = max(currentFT7, jobTable[i]->finishTime);
            sum += jobTable[i]->finishTime;
            cnt++;
        }
        else{
            sum += timer;
        }
    }
    switch(size){
        case 1:
            for(int i=0; i<6; i++){
                int idx2 = i / 2;
                if(!empty[i]){
                    currentFT2[idx2] = max(currentFT2[idx2], jobTable[i]->finishTime);
                }
            }
            for(int i=4; i<SLICE; i++){
                if(!empty[i])
                    currentFT3 = max(currentFT3, jobTable[i]->finishTime);
            }
            for(int i=0; i < SLICE; i++){
                if(empty[i]){
                    Partition p(this->id, 1, i);
                    int idx2 = i / 2;
                    if(idx2 < 3)
                        p.FT = currentFT2[idx2];
                    else
                        p.FT = currentFT3;
                    // p.FT = currentFT7;
                    part.push_back(p);
                }
            }
            break;
        case 2:
            for(int i=4; i<SLICE; i++){
                if(!empty[i])
                    currentFT3 = max(currentFT3, jobTable[i]->finishTime);
            }
            for(int i=0; i<4; i++){
                if(!empty[i])
                    currentFT4 = max(currentFT4, jobTable[i]->finishTime);
            }
            for(int i=0; i+1 < SLICE; i+=2){
                if(empty[i] && empty[i+1]){
                    Partition p(this->id, 2, i);
                    if(i < 4)
                        p.FT = currentFT4;
                    else
                        p.FT = currentFT3;
                    // p.FT = currentFT7;
                    part.push_back(p);
                }
            }
            break;
        case 3:
            for(int i=0; i<SLICE; i++){
                if(!empty[i])
                    currentFT7 = max(currentFT7, jobTable[i]->finishTime);
            }
            if(empty[4] && empty[5] && empty[6]){
                Partition p(this->id, 3, 4);
                p.FT = currentFT7;
                part.push_back(p);
            }
            break;
        case 4:
            for(int i=0; i<SLICE; i++){
                if(!empty[i])
                    currentFT7 = max(currentFT7, jobTable[i]->finishTime);
            }
            if(empty[0] && empty[1] && empty[2] && empty[3]){
                Partition p(this->id, 4, 0);
                p.FT = currentFT7;
                part.push_back(p);
            }
            break;
        default:
            cout << "Wrong partition size!\n";
            break;
    }
}

bool A100::allocate(Job *j, int size, vector<int> &slices){
    slices.clear();
    switch(size){
        case 1:
            for(int i=SLICE-1; i>=0; i--){
                if(empty[i]){
                    slices.push_back(i);
                    break;
                }
            }
            break;
        case 2:
            for(int i=4; i>=0; i-=2){
                if(empty[i] && empty[i+1]){
                    slices.push_back(i);
                    slices.push_back(i+1);
                    break;
                }
            }
            break;
        case 3:
            for(int i=4; i<7; i++){
                if(empty[i]){
                    slices.push_back(i);
                }
                else{
                    slices.clear();
                    break;
                }
            }
            break;
        case 4:
            for(int i=0; i<4; i++){
                if(empty[i]){
                    slices.push_back(i);
                }
                else{
                    slices.clear();
                    break;
                }
            }
            break;
        case 7:
            for(int i=0; i<7; i++){
                if(empty[i]){
                    slices.push_back(i);
                }
                else{
                    slices.clear();
                    break;
                }
            }
            break;
        default:
            cout << "Wrong partition size!\n";
            break;
    }
    if(slices.empty()){
        return false;
    }
    for(auto &s: slices){
        jobTable[s] = j;
        empty[s] = false;
    }
    return true;
}

void A100::allocatePart(Job *j, Partition &p, vector<int> &slices){
    for(int s=p.idx; s<p.idx + p.size; s++){
        if(!empty[s]){
            printf("zzz\n");
            exit(1);
        }
        slices.push_back(s);
        jobTable[s] = j;
        empty[s] = false;
    }
}

bool A100::hasPartition(int size){
    switch(size){
        case 1:
            for(int i=SLICE-1; i>=0; i--){
                if(empty[i]){
                    return true;
                }
            }
            break;
        case 2:
            for(int i=4; i>=0; i-=2){
                if(empty[i] && empty[i+1]){
                    return true;
                }
            }
            break;
        case 3:
            if(empty[4] && empty[5] && empty[6])
                return true;
            break;
        case 4:
            if(empty[0] && empty[1] && empty[2] && empty[3])
                return true;
            break;
        case 7:
            for(int i=0; i<7; i++){
                if(!empty[i]){
                    return false;
                }
            }
            return true;
            break;
        default:
            cout << "Wrong partition size!\n";
            break;
    }
    return false;
}

int A100::freeSliceCnt(){
    int cnt = 0;
    for(int i=0; i<SLICE; i++){
        if(empty[i])
            cnt++;
    }
    return cnt;
}