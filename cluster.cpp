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
    gpus[j->gpuID].freeSlices(j->slices);
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
    // vector<Job*> plan = AFCFS_Scheduling();
    myPlacement(plan);
}

void Cluster::bestfit(){
    vector<Job*> plan = myScheduling();
    // vector<Job*> plan = AFCFS_Scheduling();
    bestfitPlacement(plan);
}

void Cluster::worstfit(){
    vector<Job*> plan = myScheduling();
    // vector<Job*> plan = AFCFS_Scheduling();
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

vector<Job*> Cluster::AFCFS_Scheduling(){
    vector<Job*> plan;
    for(int i=PARTITION-1; i>=0; i--){
        while(!readyJobs[i].empty()){
            Job *j = readyJobs[i].top();
            readyJobs[i].pop();
            plan.push_back(j);
        }
    }
    sort(plan.begin(), plan.end(), compareArrival2());
    return plan;
}

void Cluster::myPlacement(vector<Job*> &plan){
    vector<bool> full(PARTITION, false);
    int jid = 0, N = plan.size();
    while(jid < N){
        vector<Job*> jobs;
        vector<Instance> GIs;
        int size = plan[jid]->limitSize, idx = sizeToIndex[size];
        if(full[idx]){
            readyJobs[idx].push(plan[jid++]);
            continue;
        }
        for(int i=0; i<ngpu; i++){
            gpus[i].getGI(size, timer, GIs);
        }
        if(GIs.size() == 0){
            full[idx] = true;
            readyJobs[idx].push(plan[jid++]);
            continue;
        }
        while(jid < N && plan[jid]->limitSize == size && jobs.size() < GIs.size()){
            jobs.push_back(plan[jid++]);
        }
        if(jobs.size() == GIs.size()){
            full[idx] = true;
        }
        // set finish time for sorting
        for(auto job: jobs){
            job->finishTime = timer + job->rt[idx];
        }
        stable_sort(GIs.begin(), GIs.end(), compareGI());
        stable_sort(jobs.begin(), jobs.end(), compareFT2());
        int n = GIs.size(), m = jobs.size(), i = 0, j = 0;
        while(j < m){
            vector<int> slices;
            Job *job = jobs[j];
            if(job->finishTime > GIs[i].FT[0] && m-j < n-i){
                i++;
                continue;
            }
            gpus[GIs[i].gid].allocateGI(job, GIs[i], slices, timer);
            job->run(GIs[i].gid, slices, timer);
            running_queue.push(job);
            i++;
            j++;
        }
    }
}

void Cluster::bestfitPlacement(vector<Job*> &plan){
    vector<bool> full(PARTITION, false);
    for(auto &job: plan){
        int size = job->limitSize, idx = sizeToIndex[size];
        int giID = -1;
        int minSeg = 1e9;
        vector<Instance> GIs;
        vector<int> slices;
        if(full[idx]){
            readyJobs[idx].push(job);
            continue;
        }
        for(int i=0; i<ngpu; i++){
            gpus[i].getGI(size, timer, GIs);
        }
        if(GIs.size() == 0){
            full[idx] = true;
            readyJobs[idx].push(job);
            continue;
        }
        int n = GIs.size();
        for(int i=0; i<n; i++){
            if(GIs[i].seg < minSeg){
                minSeg = GIs[i].seg;
                giID = i;
            }
        }
        if(giID == -1){
            cout<<"zzz\n";
            exit(1);
        }
        job->finishTime = timer + job->rt[idx];
        gpus[GIs[giID].gid].allocateGI(job, GIs[giID], slices, timer);
        job->run(GIs[giID].gid, slices, timer);
        running_queue.push(job);
    }
}

void Cluster::worstfitPlacement(vector<Job*> &plan){
    vector<bool> full(PARTITION, false);
    for(auto &job: plan){
        int size = job->limitSize, idx = sizeToIndex[size];
        int giID = -1;
        int maxSeg = 0;
        vector<Instance> GIs;
        vector<int> slices;
        if(full[idx])
            continue;
        for(int i=0; i<ngpu; i++){
            gpus[i].getGI(size, timer, GIs);
        }
        if(GIs.size() == 0){
            full[idx] = true;
            continue;
        }
        int n = GIs.size();
        for(int i=0; i<n; i++){
            if(GIs[i].seg > maxSeg){
                maxSeg = GIs[i].seg;
                giID = i;
            }
        }
        if(giID == -1){
            cout<<"zzz\n";
            exit(1);
        }
        job->finishTime = timer + job->rt[idx];
        gpus[GIs[giID].gid].allocateGI(job, GIs[giID], slices, timer);
        job->run(GIs[giID].gid, slices, timer);
        running_queue.push(job);
    }
}