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
    this->epoch = 0;
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
    this->pcnt = vector<int>(PARTITION);
    // vector<int> dist{10,25,45,20}, rt{66,38,29,18};
    vector<int> dist{10,25,45,20}, rt{66,33,16,8};
    int total = 0, sliceCnt = ngpu * 8, tmp = 0;
    for(int i=0; i<PARTITION; i++){
        total += dist[i] * rt[i] * indexToSize[i];
    }
    for(int i=PARTITION-1; i>=1; i--){
        int size = indexToSize[i];
        double ratio = ((double)dist[i] * rt[i] * size) / total;
        double num = sliceCnt * ratio / size;
        if(num - floor(num) > 0.5){
            pcnt[i] = (int)ceil(num);
        }
        else{
            pcnt[i] = (int)floor(num);
        }
        tmp += pcnt[i] * size;
    }
    pcnt[0] = sliceCnt - tmp;
    if(pcnt[0] <= 0){
        cout << "ccccc\n";
        exit(1);
    }
    // for(int i=0; i<PARTITION; i++){
    //     cout << pcnt[i] << "\n";
    // }
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
        logger->logUtil(timer, util);
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
        case MYALLOCATE:
            myAlgo();   // my allocation + normal placement
            break;
        case FINAL:
            final();    // my allocation + my placement
            break;
        case BEST:
            best();
            break;
        default:
            printf("Wrong algo argument!\n");
            exit(1);
            break;
    }
}

// my allocation + normal placement
void Cluster:: myAlgo(){
    vector<vector<Job*>> plan = myAllocate();
    placement(plan);
}

// my allocation + my placement
void Cluster::final(){
    vector<vector<Job*>> plan = myAllocate();
    myPlacement(plan);
}

void Cluster::best(){
    int newEpoch = timer / 500;
    int gid = 0, sidx = 0;
    if(newEpoch > epoch){
        // cout << heavy << "\n";
        // check cluster load
        int sliceCnt = 0, required = 0;
        for(int i=0; i<ngpu; i++){
            sliceCnt += gpus[i].freeSliceCnt();
        }
        for(int i=PARTITION-1; i>=0; i--){
            int size = indexToSize[i];
            required += readyJobs[i].size() * size;
        }
        if(required - sliceCnt > 0.5 * ngpu * 8){
            heavy = true;
        }
        else{
            heavy = false;
        }
        epoch = newEpoch;
    }
    if(!heavy){
        myAlgo();
    }
    else{
        for(int i=PARTITION-1; i>=0; i--){
            int size = indexToSize[i];
            for(int j=0; j<pcnt[i]; j++){
                bool valid = true;
                for(int k=0; k<size; k++){
                    if(!gpus[gid].empty[sidx+k]){
                        valid = false;
                        break;
                    }
                }
                if(!readyJobs[i].empty() && valid){
                    Job *job = readyJobs[i].top();
                    readyJobs[i].pop();
                    vector<int> slices;
                    if(!gpus[gid].allocate(job, size, slices)){
                        cout<<"zzzz\n";
                        exit(1);
                    }
                    job->run(gid, slices, timer);
                    running_queue.push(job);
                }
                sidx += size;
                if(sidx == SLICE){
                    sidx = 0;
                    gid++;
                }
            }
        }
    }
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

void Cluster::placement(vector<vector<Job*>> &plan){
    // allocate resource from large partition
    for(int i=PARTITION-1; i>=0; i--){
        int size = indexToSize[i];
        for(auto job: plan[i]){
            int gid = -1;
            int minGap = 1e9;
            for(int j=0; j<ngpu; j++){
                if(gpus[j].hasPartition(size) && gpus[j].freeSliceCnt() < minGap){
                    minGap = gpus[j].freeSliceCnt();
                    gid = j;
                }
            }
            if(gid == -1){
                cout<<"zzz\n";
                exit(1);
            }
            vector<int> slices;
            if(!gpus[gid].allocate(job, size, slices)){
                cout<<"zzzz\n";
                exit(1);
            }
            job->run(gid, slices, timer);
            running_queue.push(job);
        }
    }
}

void Cluster::myPlacement(vector<vector<Job*>> &plan){
    // 8/8 partition has no reconfiguration chance, no need to change
    int gid = 0, size = 8;
    for(auto job: plan[PARTITION-1]){
        vector<int> slices;
        while(!gpus[gid].allocate(job, size, slices)){
            gid++;
            if(gid == ngpu){
                printf("no enough resource for placement\n");
                exit(1);
            }
        }
        job->run(gid, slices, timer);
        running_queue.push(job);
        gid++;
    }
    // allocate resource from large partition
    for(int idx=PARTITION-2; idx>=0; idx--){
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
                        gpus[part1[k].gid].allocatePart(job, part1[k], slices);
                        job->run(part1[k--].gid, slices, timer);
                        running_queue.push(job);
                    }
                    continue;
                }
            }
            vector<int> slices;
            Job *job = plan[idx][j--];
            gpus[part2[i].gid].allocatePart(job, part2[i], slices);
            job->run(part2[i--].gid, slices, timer);
            running_queue.push(job);
        }
        while(j>=0){
            if(k<0){
                cout << "WTFZ\n";
            }
            vector<int> slices;
            Job *job = plan[idx][j--];
            gpus[part1[k].gid].allocatePart(job, part1[k], slices);
            job->run(part1[k--].gid, slices, timer);
            running_queue.push(job);
        }
    }
}