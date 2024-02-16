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
    vector<Job*> plan = myScheduling();
    myPlacement(plan);
}

void Cluster::bestfit(){
    vector<Job*> plan = myScheduling();
    bestfitPlacement(plan);
}

void Cluster::worstfit(){
    vector<Job*> plan = myScheduling();
    worstfitPlacement(plan);
}

vector<Job*> Cluster::myScheduling(){
    vector<Job*> plan;
    for(int i=PARTITION-1; i>=0; i--){
        int size = indexToSize[i];
        while(resource[i] != 0 && !readyJobs[i].empty()){
            Job *j = readyJobs[i].top();
            readyJobs[i].pop();
            plan.push_back(j);
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


void Cluster::myPlacement(vector<Job*> &plan){
    int idx = 0;
    while(idx < plan.size()){
        vector<Job*> jobs = {plan[idx]};
        vector<Partition> part;
        int size = plan[idx]->limitSize;
        while(idx+1 < plan.size() && plan[idx]->limitSize == size){
            jobs.push_back(plan[++idx]);
        }
        
        for(int i=0; i<ngpu; i++){
            gpus[i].getPartition(size, timer, part);
        }
        // set finish time for sorting
        for(auto job: plan[idx]){
            job->finishTime = timer + job->rt[idx];
        }
        stable_sort(part.begin(), part.end(), comparePartition());
        stable_sort(plan[idx].begin(), plan[idx].end(), compareFinish2());
        int n = part.size(), m = plan[idx].size(), i = 0, j = 0;
        while(j < m){
            vector<int> slices;
            Job *job = plan[idx][j];
            if(plan[idx][j]->finishTime > part[i].FT[0] && m-j < n-i){
                i++;
                continue;
            }
            gpus[part[i].gid].allocatePart(job, part[i], slices, timer);
            job->run(part[i].gid, slices, timer);
            running_queue.push(job);
            i++;
            j++;
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