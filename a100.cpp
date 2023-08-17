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
                    part.push_back(p);
                }
            }
            break;
        case 2:
            for(int i=0; i+1 < SLICE; i+=2){
                if(empty[i] && empty[i+1]){
                    Partition p(this->id, 2, i);
                    int idx = i / 4;
                    p.FT.push_back(currentFT4[idx]);
                    p.FT.push_back(currentFT8);
                    part.push_back(p);
                }
            }
            break;
        case 4:
            for(int i=0; i+3<SLICE; i+=4){
                if(empty[i] && empty[i+1] && empty[i+2] && empty[i+3]){
                    Partition p(this->id, 4, i);
                    p.FT.push_back(currentFT8);
                    part.push_back(p);
                }
            }
            break;
        default:
            cout << "Wrong partition size!\n";
            cout << size << "\n";
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
            for(int i=6; i>=0; i-=2){
                if(empty[i] && empty[i+1]){
                    slices.push_back(i);
                    slices.push_back(i+1);
                    break;
                }
            }
            break;
        case 4:
            for(int i=4; i>=0; i-=4){
                if(empty[i] && empty[i+1] && empty[i+2] && empty[i+3]){
                    slices.push_back(i);
                    slices.push_back(i+1);
                    slices.push_back(i+2);
                    slices.push_back(i+3);
                    break;
                }
            }
            break;
        case 8:
            for(int i=0; i<8; i++){
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
            cout << size << "\n";
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
            for(int i=6; i>=0; i-=2){
                if(empty[i] && empty[i+1]){
                    return true;
                }
            }
            break;
        case 4:
            for(int i=4; i>=0; i-=4){
                if(empty[i] && empty[i+1] && empty[i+2] && empty[i+3]){
                    return true;
                }
            }
            break;
        case 8:
            for(int i=0; i<8; i++){
                if(!empty[i]){
                    return false;
                }
            }
            return true;
            break;
        default:
            cout << "Wrong partition size!\n";
            cout << size << "\n";
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