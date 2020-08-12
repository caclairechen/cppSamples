#include <iostream>
#include <deque>
#include <list>
#include <queue>
#include <fstream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <ctime>
#include <cstdlib>
#define TIME_SLICE 5

using namespace std;

//array to store total number of each type/priority
int numPri[3];
int numType[4];

//array to store total run time used of each type/priority
int timePri[3] = {0,0,0};
int timeType[4] = {0,0,0,0};

int currTime = 0;

struct PCB{
    string name;
    int type;
    int priority;
    bool firstRun;
    int startTime;
    int timeToCom;
    int oddsOfIO;
};

queue<PCB> loadProcess(string filePath){
    //read configuration to file to load all processes to a queue
    queue<PCB> processQueue;
    ifstream file (filePath.c_str());
    string processContent;

    while (getline(file, processContent)) {
        //read line by line and create a new PCB struct by assigning corresponding fields
        istringstream iss(processContent);
        struct PCB process;
        string a, b, c, d, e;
        iss >> a >> b >> c >> d >> e;
        process.name = a;
        process.type = atoi(b.c_str());
        process.priority = atoi(c.c_str());
        process.timeToCom = atoi(d.c_str());
        process.oddsOfIO = atoi(e.c_str());
        process.firstRun = false;
        numPri[process.priority] += 1;
        numType[process.type] += 1;
        processQueue.push(process);
    }          
    return processQueue;
}

int randomNum(int max){
    //generate random number from 0 to max
    int r = rand() % (max+1);
    return r;
}

void roundRobin(queue<PCB> queue){
    //run a queue time-sliced
    int rand;
    int timeSlice;
    srand(time(0));
    while (!queue.empty()){
        struct PCB temp = queue.front();
        if(!temp.firstRun){
            temp.startTime = currTime;
            temp.firstRun = true;
        }
        //run I/O
        timeSlice = TIME_SLICE;
        rand = randomNum(100);
        if (temp.oddsOfIO < rand){
            timeSlice = randomNum(TIME_SLICE);
        }
        if (temp.timeToCom <= timeSlice){
            //finish work
            currTime += temp.timeToCom;
            temp.timeToCom = 0;
            int timeUsed = currTime - temp.startTime;
            timePri[temp.priority] += timeUsed;
            timeType[temp.type] += timeUsed;
        }
        else{
            //move to the end of queue
            currTime += timeSlice;
            temp.timeToCom -= timeSlice;
            queue.push(temp);
        }
        queue.pop();
    }
}

void shortestTime(queue<PCB> queue){
    //run a queue until completion
    while (!queue.empty()){
        struct PCB temp = queue.front();
        timePri[temp.priority] += temp.timeToCom;
        timeType[temp.type] += temp.timeToCom;
        queue.pop();
    }
}

void printTime(){
    //print average run time used
    cout << endl <<  "Average run time per priority:" << endl;
    for (int i = 0; i < 3; i++){
        cout << "Priority " << i << " average run time: " << (int)(timePri[i]/numPri[i]) << endl; 
    }
    cout << endl << "Average run time per type:" << endl;
    for (int i = 0; i < 4; i++){
        cout << "Type " << i << " average run time:" << (int)(timeType[i]/numType[i]) << endl; 
    }
}

struct PCB move(queue<PCB> &from, queue<PCB> &to){
    //copy from queue to to queue, and return the process with min length
    struct PCB shortest;
    if(!from.empty()){
        shortest = from.front();
        from.pop();
    }
    while (!from.empty()){
        if(from.front().timeToCom < shortest.timeToCom){
            to.push(shortest);
            shortest = from.front();
        }
        else{
            to.push(from.front());
        }
        from.pop();
    }
    return shortest;
}

void sortQueue(queue<PCB> &q){
    //sort the queue based on shortest time to completion
    queue<PCB> myQueue1;
    queue<PCB> myQueue2;
    bool from1 = true;
    struct PCB shortestProcess;
    while(!q.empty()){
        myQueue1.push(q.front());
        q.pop();
    }
    while(!myQueue1.empty() || !myQueue2.empty()){
        if(from1){
            shortestProcess = move(myQueue1, myQueue2);
            q.push(shortestProcess);
            from1 = false;
        }
        else{
            shortestProcess = move(myQueue2, myQueue1);
            q.push(shortestProcess);
            from1 = true;
        }
    }
}

int main(int argc, char **argv){
    int schePolicy;
    queue<PCB> processQueue;
    processQueue = loadProcess("processes.txt");

    if (argc != 2){
        fprintf(stderr, "Usage: %s <integer> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    schePolicy = atoi(argv[1]);

    if(schePolicy == 1){
        //First Come First Serve
        cout <<  "Using pure round-robin" << endl;
        roundRobin(processQueue);
        printTime();
    }
    else if(schePolicy == 2){
        //Shortest Time To Completion
        cout <<  "Using shortest time to completion first" << endl;
        sortQueue(processQueue);
        shortestTime(processQueue);
        printTime();
    }
    else if(schePolicy == 3){
        //Priority Round Robin
        cout <<  "Using priority round-robin" << endl;
        queue<PCB> priQueue[3];
        while(!processQueue.empty()){
            //each queue contains a priority level
            priQueue[processQueue.front().priority].push(processQueue.front());
            processQueue.pop();
        }
        for(int i = 0; i < 3; i++){
            //run each queue
            roundRobin(priQueue[i]);
        }
        printTime();
    }
    else{
        cout <<  "Expect 1, 2, 3" << endl;
    }
    return 0;
}
