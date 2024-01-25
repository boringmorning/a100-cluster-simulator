#include "cluster.h"

const int indexToSize[] = {1,2,4,8};
unordered_map<int,int> sizeToIndex = {
    {1,0},
    {2,1},
    {4,2},
    {8,3}
};

Cluster::Cluster(){
    this->timer = 0;
    this->util = 0.0;
    this->ngpu = 0;
    this->readyJobs = vector<priority_queue<Job*, vector<Job*>, compareArrival>>(PARTITION);
}


Cluster::Cluster(int ngpu, Logger *logger, int algo){
    this->heavy = false;
    this->timer = 0;
    this->util = 0.0;
    this->ngpu = ngpu;
    this->logger = logger;
    this->algo = algo;
    this->resource = vector<int>(PARTITION);
    this->readyJobs = vector<priority_queue<Job*, vector<Job*>, compareArrival>>(PARTITION);
    for(int i=0; i<ngpu; i++){
        gpus.push_back(A100(i));
    }
}

void Cluster::newJob(Job *j){
    jobs.push_back(j);
    job_queue.push(j);
}

void Cluster::run(){
    int readyCnt = 0;
    for(int i=0; i<PARTITION; i++){
        readyCnt += readyJobs[i].size();
    } 
    while(!(job_queue.empty() && running_queue.empty() && readyCnt == 0)){
        tt nextFinishTime = running_queue.empty()? 1e9 : running_queue.top()->finishTime;
        tt nextArrivalTime = job_queue.empty()? 1e9 : job_queue.front()->arrivalTime;
        // jump to next event
        timer = std::min(nextFinishTime, nextArrivalTime);
        updateStatus();
        schedule();
        readyCnt = 0;
        for(int i=0; i<PARTITION; i++){
            readyCnt += readyJobs[i].size();
        }
        double inUse = 0.0;
        for(int i=0; i<ngpu; i++){
            for(int j=0; j<SLICE; j++){
                if(!gpus[i].empty[j]){
                    inUse++;
                }
            }
        }
        double util = inUse / (ngpu * SLICE);
        logger->logUtil(timer, util, readyCnt);
    }
    logger->end(timer);
}

void Cluster::updateStatus(){
    while(!running_queue.empty() && running_queue.top()->finishTime <= timer){
        Job *j = running_queue.top();
        running_queue.pop();
        finishJob(j);
    }
    while(!job_queue.empty() && job_queue.front()->arrivalTime <= timer){
        Job *j = job_queue.front();
        job_queue.pop();
        int idx = sizeToIndex[j->limitSize];
        readyJobs[idx].push(j);
    }
    for(int i=0; i<PARTITION; i++){
        resource[i] = 0;
    }
    for(int i=0; i<ngpu; i++){
        gpus[i].getResource(resource);
    }
    
}

void Cluster::finishJob(Job *j){
    gpus[j->gpuID].freePartition(j->slices);
    logger->finishJob(j);
    // delete(j);
}

void Cluster::schedule(){
    switch(algo){
        case MYALGO:
            myAlgo();
            break;
        case BESTFIT:
            bestfit();
            break;
        case WORSTFIT:
            worstfit();
            break;
        default:
            printf("Wrong algo argument!\n");
            exit(1);
            break;
    }
}

void Cluster:: myAlgo(){
    vector<vector<Job*>> plan = myAllocate();
    myPlacement(plan);
}

void Cluster::bestfit(){
    vector<vector<Job*>> plan = myAllocate();
    bestfitPlacement(plan);
}

void Cluster::worstfit(){
    vector<vector<Job*>> plan = myAllocate();
    worstfitPlacement(plan);
}

bool Cluster::validScaleUp(int newSize){
    switch(newSize){
        case 2:
            if(resource[0] >= 1 && resource[1] >= 1)
                return true;
            break;
        case 4:
            if(resource[0] >= 2 && resource[1] >= 1 && resource[2] >= 1){
                return true;
            }
            break;
        case 8:
            if(resource[0] >= 4 && resource[1] >= 2 && resource[2] >= 1 && resource[3] >= 1){
                return true;
            }
            break;
        default:
            printf("wrong scale up size!\n");
            exit(1);
            break;
    }
    return false;
}

vector<vector<Job*>> Cluster::myAllocate(){
    vector<vector<Job*>> plan(PARTITION);
    for(int i=PARTITION-1; i>=0; i--){
        int size = indexToSize[i];
        while(resource[i] != 0 && !readyJobs[i].empty()){
            Job *j = readyJobs[i].top();
            readyJobs[i].pop();
            plan[i].push_back(j);
            switch(size){
                case 1:
                    resource[0]--;
                    break;
                case 2:
                    resource[0]-=2;
                    resource[1]--;
                    break;
                case 4:
                    resource[0]-=4;
                    resource[1]-=2;
                    resource[2]--;
                    break;
                case 8:
                    resource[0]-=8;
                    resource[1]-=4;
                    resource[2]-=2;
                    resource[3]--;
                    break;
            }
        }
    }
    return plan;
}

void Cluster::myPlacement(vector<vector<Job*>> &plan){
    // allocate resource from large partition
    for(int idx=PARTITION-1; idx>=0; idx--){
        int size = indexToSize[idx];
        vector<Partition> part, part1, part2;
        for(int i=0; i<ngpu; i++){
            gpus[i].getPartition(size, timer, part);
        }
        for(auto &p: part){
            if(p.FT[0] == timer){
                part1.push_back(p);
            }
            else{
                part2.push_back(p);
            }
        }
        // set finish time for sorting
        for(auto job: plan[idx]){
            job->finishTime = timer + job->rt[idx];
        }
        stable_sort(part2.begin(), part2.end(), comparePartition());
        stable_sort(part1.begin(), part1.end(), comparePartition());
        sort(plan[idx].begin(), plan[idx].end(), compareFinish2());
        int i = part2.size()-1, j = plan[idx].size()-1, k = part1.size()-1;
        while(i >= 0 && j>=0){
            if(j>0 && k>0){
                int gap = plan[idx][j]->finishTime - timer;
                int gap2;
                if(i == 0){
                    gap2 = max(plan[idx][j]->finishTime - part2[i].FT[0], 0) + (plan[idx][j-1]->finishTime - timer);
                }
                else{
                    gap2 = max(plan[idx][j]->finishTime - part2[i].FT[0], 0) + max(plan[idx][j-1]->finishTime - part2[i-1].FT[0], 0);
                }
                if(gap < gap2){
                    for(int c=0; c<2; c++){
                        vector<int> slices;
                        Job *job = plan[idx][j--];
                        gpus[part1[k].gid].allocatePart(job, part1[k], slices, timer);
                        job->run(part1[k--].gid, slices, timer);
                        running_queue.push(job);
                    }
                    continue;
                }
            }
            vector<int> slices;
            Job *job = plan[idx][j--];
            gpus[part2[i].gid].allocatePart(job, part2[i], slices, timer);
            job->run(part2[i--].gid, slices, timer);
            running_queue.push(job);
        }
        while(j>=0){
            if(k<0){
                cout << "WTFZ\n";
            }
            vector<int> slices;
            Job *job = plan[idx][j--];
            gpus[part1[k].gid].allocatePart(job, part1[k], slices, timer);
            job->run(part1[k--].gid, slices, timer);
            running_queue.push(job);
        }
    }
}

void Cluster::bestfitPlacement(vector<vector<Job*>> &plan){
    // allocate resource from large partition
    for(int idx=PARTITION-1; idx>=0; idx--){
        int size = indexToSize[idx];
        for(auto job: plan[idx]){
            int pid = -1;
            int minSeg = 1e9;
            vector<Partition> part;
            vector<int> slices;
            for(int i=0; i<ngpu; i++){
                gpus[i].getPartition(size, timer, part);
            }
            int n = part.size();
            for(int i=0; i<n; i++){
                if(part[i].seg < minSeg){
                    minSeg = part[i].seg;
                    pid = i;
                }
            }
            if(pid == -1){
                cout<<"zzz\n";
                exit(1);
            }
            job->finishTime = timer + job->rt[idx];
            gpus[part[pid].gid].allocatePart(job, part[pid], slices, timer);
            job->run(part[pid].gid, slices, timer);
            running_queue.push(job);
        }
    }
}

void Cluster::worstfitPlacement(vector<vector<Job*>> &plan){
    // allocate resource from large partition
    for(int idx=PARTITION-1; idx>=0; idx--){
        int size = indexToSize[idx];
        for(auto job: plan[idx]){
            int pid = -1;
            int maxSeg = 0;
            vector<Partition> part;
            vector<int> slices;
            for(int i=0; i<ngpu; i++){
                gpus[i].getPartition(size, timer, part);
            }
            int n = part.size();
            for(int i=0; i<n; i++){
                if(part[i].seg > maxSeg){
                    maxSeg = part[i].seg;
                    pid = i;
                }
            }
            if(pid == -1){
                cout<<"zzz\n";
                exit(1);
            }
            job->finishTime = timer + job->rt[idx];
            gpus[part[pid].gid].allocatePart(job, part[pid], slices, timer);
            job->run(part[pid].gid, slices, timer);
            running_queue.push(job);
        }
    }
}