#include "cluster.h"

const int indexToSize[] = {1,2,3,4,7};
unordered_map<int,int> sizeToIndex = {
    {1,0},
    {2,1},
    {3,2},
    {4,3},
    {7,4}
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
        // cout << running_queue.size() << "\n";
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
        case NOPART:
            noPart();
            break;
        case STATICPART:
            staticPart();
            break;
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
    // cout << running_queue.size() << "\n";
}

// each job will use a full a100 card.
void Cluster::noPart(){
    // int idx = sizeToIndex[7];
    // int n = min((int)readyJobs.size(), resource[idx]);
    // vector<vector<Job*>> plan(PARTITION);
    // for(int i=0; i<n; i++){
    //     Job *j = readyJobs.top();
    //     readyJobs.pop();
    //     plan[idx].push_back(j);
    // }
    // placement(plan);
}

// each job will use a 1/7 partition.
void Cluster::staticPart(){
    // int idx = sizeToIndex[1];
    // int n = min((int)readyJobs.size(), resource[idx]);
    // vector<vector<Job*>> plan(PARTITION);
    // for(int i=0; i<n; i++){
    //     Job *j = readyJobs.top();
    //     readyJobs.pop();
    //     plan[idx].push_back(j);
    // }
    // placement(plan);
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
    
    int n = readyJobs.size(), idx;
    vector<vector<Job*>> plan(PARTITION);
    priority_queue<JobMetrics, vector<JobMetrics>, compareMetrics> pq;
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
    // if(n >= resource[sizeToIndex[1]]){
    //     idx = sizeToIndex[1];
    //     n = resource[idx];
    //     for(int i=0; i<n; i++){
    //         Job *j = readyJobs.top();
    //         readyJobs.pop();
    //         plan[idx].push_back(j);
    //     }
    // }
    // else{
    //     if(n <= resource[sizeToIndex[7]]){
    //         idx = sizeToIndex[7];
    //         for(int i=0; i<n; i++){
    //             Job *j = readyJobs.top();
    //             readyJobs.pop();
    //             plan[idx].push_back(j);
    //         }
    //     }
    //     else{
    //         priority_queue<JobMetrics, vector<JobMetrics>, compareMetrics> pq;
    //         for(int i=0; i<n; i++){
    //             Job *j = readyJobs.top();
    //             readyJobs.pop();
    //             JobMetrics m(j, 1);
    //             pq.push(m);
    //         }
    //         resource[0] -= n;
    //         while(!pq.empty()){
    //             JobMetrics m = pq.top();
    //             pq.pop();
    //             bool fixed = true;
    //             switch(m.size){
    //                 case 1:
    //                     if(validScaleUp(2)){
    //                         resource[0] -= 1;
    //                         resource[1] -= 1;
    //                         m.size = 2;
    //                         if(validScaleUp(3) || validScaleUp(4)){
    //                             m.updateMetrics();
    //                             pq.push(m);
    //                             fixed = false;
    //                         }
    //                     }
    //                     break;
    //                 case 2:
    //                     if(validScaleUp(4)){
    //                         resource[0] -= 2;
    //                         resource[1] -= 1;
    //                         resource[3] -= 1;
    //                         m.size = 4;
    //                         if(validScaleUp(7)){
    //                             m.updateMetrics();
    //                             pq.push(m);
    //                             fixed = false;
    //                         }
    //                     }
    //                     else if(validScaleUp(3)){
    //                         resource[0] -= 1;
    //                         resource[2] -= 1;
    //                         m.size = 3;
    //                     }
    //                     break;
    //                 case 4:
    //                     if(validScaleUp(7)){
    //                         resource[0] -= 3;
    //                         resource[1] -= 1;
    //                         resource[2] -= 1;
    //                         resource[4] -= 1;
    //                         m.size = 7;
    //                     }
    //                     break;
    //                 default:
    //                     printf("wrong scale up size!\n");
    //                     exit(1);
    //                     break;
    //             }
    //             if(fixed){
    //                 int idx = sizeToIndex[m.size];
    //                 plan[idx].push_back(m.j);
    //             }
    //         }
    //     }
    // }
    return plan;
}

void Cluster::placement(vector<vector<Job*>> &plan){
    // allocate resource from large partition
    for(int i=PARTITION-1; i>=0; i--){
        int size = indexToSize[i];
        // sort(plan[i].begin(), plan[i].end(), compareFinish());
        for(auto job: plan[i]){
            int gid = -1;
            int minGap = 1e9;
            for(int j=0; j<ngpu; j++){
                if(gpus[j].hasPartition(size) && gpus[j].freeSliceCnt() - size < minGap){
                    minGap = gpus[j].freeSliceCnt() - size;
                    gid = j;
                }
            }
            if(gid == -1)
                break;
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
    }
}

void Cluster::myPlacement(vector<vector<Job*>> &plan){
    // 7/7 partition has no reconfiguration chance, no need to change
    int gid = 0, size = 7;
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
        vector<Partition> part;
        for(int i=0; i<ngpu; i++){
            gpus[i].getPartition(size, part);
        }
        // set finish time for sorting
        for(auto job: plan[idx]){
            job->finishTime = timer + job->rt[idx];
        }
        stable_sort(part.begin(), part.end(), comparePartition());
        sort(plan[idx].begin(), plan[idx].end(), compareFinish2());
        int n = part.size(), m = plan[idx].size();
        vector<vector<int>> dp(m+1, vector<int>(n+1, 0));
        if(n < m){
            printf("zz\n");
            exit(1);
        }
        for(int j=0; j<m; j++){
            for(int i=j; i<n; i++){
                int waste = min(plan[idx][j]->finishTime - part[i].FT, 0);
                dp[j+1][i+1] = dp[j][i] + waste;
                if(i > j)
                    dp[j+1][i+1] = min(dp[j+1][i+1], dp[j+1][i]);
            }
        }
        // backtracking
        int i = n-1, j = m-1;
        while(j >= 0){
            if(i == j || dp[j+1][i+1] != dp[j+1][i]){
                vector<int> slices;
                Job *job = plan[idx][j];
                gpus[part[i].gid].allocatePart(job, part[i], slices);
                job->run(part[i].gid, slices, timer);
                running_queue.push(job);
                i--;
                j--;
            }
            else{
                i--;
            }
        }
    }
}