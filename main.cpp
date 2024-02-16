#include "cluster.h"

extern unordered_map<int,int> sizeToIndex;

void readJobs(Cluster &cluster, string fname){
    ifstream file;
    string line, tmp;
    file.open(fname);
    int jobID = 0;
    while(getline(file, line))
    {
        // arrive time
        size_t pos = 0, last = 0;
        pos = line.find(" ", last);
        tmp = line.substr(last, pos - last);
        Job *job = new Job(jobID++, stoi(tmp));
        last = pos+1;
        for(int i=0; i<PARTITION; i++){
            pos = line.find(" ", last);
            tmp = line.substr(last, pos - last);
            job->rt[i] = stoi(tmp);
            last = pos+1;
        }
        pos = line.find(" ", last);
        tmp = line.substr(last, pos - last);
        job->limitSize = stoi(tmp);
        job->speed = job->rt[sizeToIndex[job->limitSize]];
        cluster.newJob(job);
    }
    file.close();
}

int main(int argc, char *argv[]) {
    if(argc != 5){
        printf("Must have 4 args!\n");
    }
    int ngpu, algo;
    string jobFile, outFile, configID, testcaseID, postfix;
    
    ngpu = stoi(argv[1]);
    configID = argv[2];
    testcaseID = argv[3];
    algo = stoi(argv[4]);

    switch(algo){
        case MYALGO:
            postfix = string("my");
            break;
        case BESTFIT:
            postfix = string("bf");
            break;
        case WORSTFIT:
            postfix = string("wf");
            break;
        default:
            printf("wrong algo argument!\n");
            exit(1);
    }
    jobFile = string("workload/load") + configID + string("_") + testcaseID + string(".txt");
    outFile = string("report/load") + configID + string("_") + testcaseID + string("_") + postfix + string(".txt");

    Logger logger(outFile);
    Cluster cluster(ngpu, &logger, algo);
    readJobs(cluster, jobFile);
    cluster.run();
    return 0;
}