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
    this->ngpu = 0;
    this->readyJobs = vector<priority_queue<Job*, vector<Job*>, compareSpeed>>(PARTITION);
}


Cluster::Cluster(int ngpu, Logger *logger, int algo){
    this->timer = 0;
    this->ngpu = ngpu;
    this->logger = logger;
    this->algo = algo;
    this->resource = vector<int>(PARTITION);
    this->readyJobs = vector<priority_queue<Job*, vector<Job*>, compareSpeed>>(PARTITION);
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

bool Cluster::validScaleUp(int newSize){
    switch(newSize){
        case 2:
            if(resource[0] >= 1 && resource[1] >= 1)
                return true;
            break;
        case 3: // 2->3
            if(resource[0] >= 1 && resource[2] >= 1){
                return true;
            }
            break;
        case 4: // 2->4, tricky here
            if(resource[0] >= 2 && resource[1] >= 1 && resource[3] >= 1){
                return true;
            }
            break;
        case 7:
            if(resource[0] >= 3 && resource[1] >= 1 && resource[2] >= 1 && resource[4] >= 1){
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
                case 3:
                    resource[0]-=3;
                    resource[1]--;
                    resource[2]--;
                    break;
                case 4:
                    resource[0]-=4;
                    resource[1]-=2;
                    resource[3]--;
                    break;
                case 7:
                    resource[0]-=7;
                    resource[1]-=3;
                    resource[2]--;
                    resource[3]--;
                    resource[4]--;
                    break;
            }
        }
    }
    // for(int idx=PARTITION-1; idx>=0; idx--){
    //     cout << plan[idx].size() << "\n";
    // }
    // cout << "\n";
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
                if(gpus[j].hasPartition(size) && gpus[j].freeSliceCnt() - size < minGap){
                    minGap = gpus[j].freeSliceCnt() - size;
                    gid = j;
                }
            }
            if(gid == -1){
                cout<<"zz\n";
                exit(1);
            }
            vector<int> slices;
            if(!gpus[gid].allocate(job, size, slices)){
                cout<<"zz\n";
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
    }
    // allocate resource from large partition
    for(int idx=PARTITION-2; idx>=0; idx--){
        int size = indexToSize[idx];
        vector<Partition> part, part2;
        for(int i=0; i<ngpu; i++){
            gpus[i].getPartition(size, timer, part, part2);
        }
        // set finish time for sorting
        for(auto job: plan[idx]){
            job->finishTime = timer + job->rt[idx];
        }
        stable_sort(part.begin(), part.end(), comparePartition());
        sort(plan[idx].begin(), plan[idx].end(), compareFinish2());
        int n = part.size(), m = plan[idx].size();
        if(n < m){
            printf("zz\n");
            exit(1);
        }
        for(int j=0; j<m; j++){
            int pid = n-m+j;
            vector<int> slices;
            Job *job = plan[idx][j];
            gpus[part[pid].gid].allocatePart(job, part[pid], slices);
            job->run(part[pid].gid, slices, timer);
            running_queue.push(job);
            // cout << part[pid].FT[0] << "\n";
        }
        // cout << "\n";
    }
}