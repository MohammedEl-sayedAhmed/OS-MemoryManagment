#include "headers.h"
#include "linkedList.h"
#include "PriorityQueue.h"
#include "Queue.h"
#include "PCB.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <math.h>


void userHandler(int signum);
void startProcess(PCB* processPCB, FILE* outLogFile);
void resumeProcess(PCB* processPCB, FILE* outLogFile, bool silent);
void stopProcess(PCB* processPCB, FILE* outLogFile, bool silent);
void finishProcess(PCB* processPCB, FILE* outLogFile,FILE* outMemFile);
void SRTN(FILE* outLogFile, FILE* outMemFile);
void HPF(FILE* outLogFile, FILE* outMemFile);
void RR(FILE* outLogFile, FILE* outMemFile, int Quantum);
int succesful_exit_handler = 0;   // global variable to store that the child exited successfully
int finish_scheduler = 0;         // global variable to store if the scheduler should stop (No other processes)

// global variables initialized for calculations
double total_waiting_time = 0;
double total_proceesing_time = 0;
double total_WTA = 0;
double processCount = 0;
Queue WTAQueue;
void StandardDeviation(FILE* outCalcFile);

// array of memory chunks linked lists
LinkedList freeChunks[8];
// allocates memory for the given process
void allocate_mem(PCB* processPCB, FILE* outMemFile);
// gets the first available chunk in the memory list with number list_num
int getFirstFreeChunk(int list_num);
// Adds the required chunk number to the free chuncks
int freeChunck(int list_num, int chunk_num);
// Merge free memory chunks
int mergeFreeChuncks(int list_num, int freed_chunk_num);
//printing free chuncks lists
void DisplayChunkLists();


int main(int argc, char * argv[])
{

    // Establish communication with the clock module
    initClk();

    // Handler for SIGUSR1 signal sent by children to the scheduler upon successful termination
    signal(SIGUSR1, userHandler);
    
  
    // Open an output file for the scheduler log (in the write mode)
    FILE* outLogFile = (FILE *) malloc(sizeof(FILE));
    outLogFile = fopen("SchedulerLog.txt", "w");
    if (outLogFile == NULL) {
        printf("Could not open output file for scheduler log.\n");
    }

    // Open an output file for the scheduler calculations (in the write mode)
    FILE* outCalcFile = (FILE *) malloc(sizeof(FILE));
    outCalcFile = fopen("SchedulerCalc.txt", "w");
    if (outCalcFile == NULL) {
        printf("Could not open output file for scheduler calculations.\n");
    }

    // Open an output file for the memory log (in the write mode)
    FILE* outMemFile = (FILE *) malloc(sizeof(FILE));
    outMemFile = fopen("MemoryLog.txt", "w");
    if (outMemFile == NULL) {
        printf("Could not open output file for memory log.\n");
    } 

    // Read the passed arguments 
    int quantum;
    char *schedalg = NULL;
    if (argc == 2) {

        // Get the chosen algorithm 
        schedalg = argv[0];
        ///////printf("Algorithm chosen: %s.\n", schedalg); 

        // Get the quantum needed for round robin algorithm 
        quantum = atoi(argv[1]);
        ///////printf("Quantum chosen: %d.\n", quantum);
    }

    // Initialize message queue 
    initMsgQueue();

    // Initialize linked lists of free memory chunks
    for (int i = 0; i < 8; i++) {
        LLInit(&(freeChunks[i]), sizeof(int));
    }
    SortedInsert(&(freeChunks[7]), 1);

    // Initialize queue of weighted turnaround times
    queueInit(&WTAQueue, sizeof(double));

    // Get initial value of CPU clocks used by scheduler
    clock_t befClocks = clock();
    ///////printf("Initial value of scheduler CPU clocks %ld\n", befClocks);

    // Run the chosen algorithm (HPF, RR or SRTN)
    if(strcmp(schedalg,"HPF") == 0)
    {
        printf("Chosen algorithm is HPF.\n");
        HPF(outLogFile,outMemFile);  
    }
    else if (strcmp(schedalg,"RR") == 0) {
        printf("Chosen algorithm is RR with a quantum of %d seconds.\n", quantum);
        RR(outLogFile,outMemFile,quantum);
    }
    else if (strcmp(schedalg,"SRTN") == 0) {
        printf("Chosen algorithm is SRTN.\n");
        SRTN(outLogFile,outMemFile);
    }

    // Get final value of CPU clocks used by scheduler
    clock_t aftClocks = clock();
    ///////printf("Final value of scheduler CPU clocks %ld\n", aftClocks);

    // Total CPU clocks used by scheduler
    clock_t totalClks = aftClocks - befClocks;

    // Calculate total time taken by scheduler in seconds
    double totalSched = ((double) totalClks) + (((double) total_proceesing_time) * ((double) CLOCKS_PER_SEC));
    totalSched = (double) (totalSched/CLOCKS_PER_SEC);
    
    // Calculate CPU utilization and print it in output file
    double cpu_utilization= (double) ((total_proceesing_time/totalSched)*100);
    fprintf(outCalcFile, "CPU  utilization: %.0f %% \n", cpu_utilization);
    
    // Calculate average weighted turnaround time and print it in output file
    double AWTA = (double) (total_WTA/processCount);
    fprintf(outCalcFile,"Avg WTA = %.2f\n", AWTA);

    // Calculate average waiting time and print it in output file
    double Avg_waiting= (double) (total_waiting_time/processCount);
    fprintf(outCalcFile,"Avg Waiting = %.2f \n",Avg_waiting);

    // Calculate standard deviation and print it in output file
    StandardDeviation(outCalcFile);

    // Close the output log and calculations file
    fclose(outLogFile);
    fclose(outCalcFile);
    fclose(outMemFile);
    
    // Upon termination, release resources of communication with the clock module
    //destroyClk(false);

    return 0;
}

// Allocates memory for the given process
void allocate_mem(PCB* processPCB,FILE* outMemFile) {
    // get the number of the list corresponding to the required memory size
    int list_num = ceil(log2(processPCB->mem_size)) - 3;
    int chunk_size = pow(2, (list_num + 3));    
    int chunk_num = getFirstFreeChunk(list_num);

    // update process PCB to store number of allocated memory chunk
    processPCB->alloc_mem_chunk = chunk_num;

    //print for testing
    //printf("\n\nThe process with ID %d is allocated at %d, and its chunck number is %d\n\n", processPCB->id, getClk(), processPCB->alloc_mem_chunk);
    //DisplayChunkLists();

    int currTime1 = getClk();
    int startingbyte = chunk_size*(chunk_num-1);
    int endbyte = startingbyte + chunk_size -1;

    fprintf(outMemFile, "At time %d allocated %d bytes for process %d from %d to %d\n", currTime1, processPCB->mem_size, processPCB->id, startingbyte, endbyte) ;

}

// Gets the first available chunk in the memory list with number list_num
int getFirstFreeChunk(int list_num) {
    
    if ((list_num < 0) || (list_num > 7)) {
        printf("Required chunk size is invalid; out of boundaries.\n");
        return -1;
    }

    if (freeChunks[list_num].sizeOfLL != 0) {
        return (delete_begin((&freeChunks[list_num])));
    }
    else {
        if ((list_num + 1) > 7) {
            printf("No free memory available for allocation.\n");
            return -1;
        }

        int chunk_num = getFirstFreeChunk(list_num + 1);

        if (chunk_num != -1) 
        {
            SortedInsert((&freeChunks[list_num]), chunk_num*2);
            return ((chunk_num*2) - 1);
        }
        else {
            return -1;
        }
    }
}

// Adds the required chunk number to the free chuncks
int freeChunck(int list_num, int chunk_num)
{
    //this function checks if the required chunk number is not free, then add it to the free chuncks
    
    if (chunk_num == 0)
    {
        printf("Invalid chunck num");
        return 0;
    }

    if ((list_num < 0) || (list_num > 7)) {
        printf("Invalid chunk size, No memory is allocated with this chunk.\n");
        return 0;
    }
    else
    {
        //check if this chunck is free
        int found = findPos(&freeChunks[list_num], chunk_num);

        //if it is already free, we cannot deallocate it
        if(found == 1)
        {
            printf("There is no memory allocated in this chunck number");
            return 0;
        }
        //if it is not free, we add it to the free chuncks
        else
        {
            SortedInsert(&freeChunks[list_num],chunk_num);
            return chunk_num;
        }   
    }
}

// Merge free memory chunks
int mergeFreeChuncks(int list_num, int freed_chunk_num)
{
    if (freed_chunk_num == 0)
    {
        printf("\nInvalid chunck num\n");
        return 0;
    }
    if ((list_num < 0) || (list_num > 7)) {
        printf("\nInvalid chunk size, No memory is allocated with this chunk.\n");
        return 0;
    }

    //check if the preceeding or proceeding elements are found (even or odd)
    int oddChunck = 1;   //0 if even, 1 if odd
    int found;

    if (freed_chunk_num % 2 == 0)
    {
        found = findPos(&freeChunks[list_num],freed_chunk_num-1);
        oddChunck = 0;
    }
    else
    {
        found = findPos(&freeChunks[list_num], freed_chunk_num+1);
        oddChunck = 1;
    }

    //if element is not found, return as there is no merge 
    if (found == 0)
    {
        printf("\nFound no element suitable for merging\n");
        return 0;
    }
    else
    {
        if (oddChunck == 0)  //even chunck 
        {
            int del = delete_element(&freeChunks[list_num], freed_chunk_num);
            int del_prev = delete_element(&freeChunks[list_num], freed_chunk_num-1);
            if (del == 1 && del_prev == 1) //successful deletion
            {
                SortedInsert(&freeChunks[list_num+1], freed_chunk_num/2);
                return (1+mergeFreeChuncks(list_num+1, freed_chunk_num/2));
            }
            else
            {
                printf("\nUnable to merge\n");
                return 0;
            }  
        }
        else   //odd chunck
        {
            int del_next = delete_element(&freeChunks[list_num], freed_chunk_num+1);
            //printf("\nDeleted Element: %d", del_next);
            int del = delete_element(&freeChunks[list_num], freed_chunk_num);
            //printf("\nDeleted Element: %d", del);
            if (del == 1 && del_next == 1) //successful deletion
            {
                SortedInsert(&freeChunks[list_num+1], (freed_chunk_num+1)/2);
                return (1+ mergeFreeChuncks(list_num+1, (freed_chunk_num+1)/2));
            }
            else
            {
                printf("\nUnable to merge\n");
                return 0;
            } 
        }
    }
}

// Resumes a currently running process
void resumeProcess(PCB* processPCB, FILE* outLogFile, bool silent) {

    // Send a continue signal to the process
    kill(processPCB->pid, SIGCONT);

    // Calculate and update the process waiting time and state
    int currTime = getClk();
    processPCB->waitingTime = (currTime - processPCB->arrivalTime) - (processPCB->runTime - processPCB->remainingTime);
    processPCB->state = RUNNING;

    // Print the "resumed" line in the output log file
    if (!silent) {
        fprintf(outLogFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", currTime, processPCB->id, processPCB->arrivalTime, processPCB->runTime, processPCB->remainingTime, processPCB->waitingTime);
    }
}

// Starts a process for the first time
void startProcess(PCB* processPCB, FILE* outLogFile) {

    // Create the process
    int pid = fork();

    if (pid == -1)
        perror("Error in fork. Could not start process.\n");

    else if (pid == 0)
    {
        char str[100];
        sprintf(str, "%d", processPCB->runTime);
        char * param[] = {str, NULL};
        execv("./process.out", param); // param contains the running time of the process
    }
    else {
        int currTime = getClk();

        // Update the process PCB fields as appropriate
        processPCB->pid = pid;
        processPCB->startTime = currTime;
        processPCB->remainingTime = processPCB->runTime;
        processPCB->waitingTime = currTime - processPCB->arrivalTime;
        processPCB->state = RUNNING;
        ///////printf("Process created successfully.\n");

        // Print the "started" line in the output log file
        fprintf(outLogFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", currTime, processPCB->id, processPCB->arrivalTime, processPCB->runTime, processPCB->remainingTime, processPCB->waitingTime);
    }
}

// Stops a currently running process
void stopProcess(PCB* processPCB, FILE* outLogFile, bool silent) {

    // Send a stop signal to the process
    kill(processPCB->pid, SIGSTOP);

    // Calculate and update the process remaining time and state
    int currTime = getClk();
    processPCB->remainingTime = (processPCB->runTime) -  (currTime - processPCB->arrivalTime - processPCB->waitingTime);
    processPCB->state = WAITING;

    // Print the "stopped" line in the output log file
    if(!silent) {
        fprintf(outLogFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", currTime, processPCB->id, processPCB->arrivalTime, processPCB->runTime, processPCB->remainingTime, processPCB->waitingTime);
    }
}

// Updates the process data as appropriate upon termination
void finishProcess(PCB* processPCB, FILE* outLogFile,FILE* outMemFile)
{
    int currTime = getClk();
    //free memory chunck and merge 
    int list_num = ceil(log2(processPCB->mem_size)) - 3;
    int chunk_size = pow(2, (list_num + 3));    
    int merge = mergeFreeChuncks(list_num,freeChunck(list_num,processPCB->alloc_mem_chunk));

    
    
    // Calculate and update the process remaining time, finish time and state
    processPCB->remainingTime = 0;
    processPCB->finishTime = processPCB->arrivalTime + processPCB->waitingTime + processPCB->runTime;
    processPCB->state = FINISHED;

    //print for testing
    //printf("\n\nThe process with ID %d is deallocated at %d, and its chunck number is %d\n\n",processPCB->id, processPCB->finishTime, processPCB->alloc_mem_chunk);
    //DisplayChunkLists();

    // Calculate turnaround time and the weighted turnaround time
    double turn_around_time = currTime - processPCB->arrivalTime;
    double w_turn_around_time = turn_around_time/((double)(processPCB->runTime));

    // Calcualate contribution to the global variables
    total_waiting_time = total_waiting_time + processPCB->waitingTime;
    total_WTA = total_WTA + w_turn_around_time;
    total_proceesing_time = total_proceesing_time + processPCB->runTime;
    processCount++;
    double* WTApointer = (double *) malloc(sizeof(double));
    *WTApointer =  w_turn_around_time;
    enqueue(&WTAQueue, WTApointer);

    // Print the "finished" line in the output log file
    fprintf(outLogFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %.0f WTA %.2f\n", currTime, processPCB->id, processPCB->arrivalTime, processPCB->runTime, processPCB->remainingTime, processPCB->waitingTime, turn_around_time, w_turn_around_time);
   ////////////// 

    int startingbyte = chunk_size*(processPCB->alloc_mem_chunk-1);
    int endbyte = startingbyte + chunk_size -1;

    fprintf(outMemFile, "At time %d freed %d bytes from process %d from %d to %d\n", currTime, processPCB->mem_size, processPCB->id, startingbyte, endbyte) ;

/////////////////////
    free(processPCB);
    processPCB = NULL;
    succesful_exit_handler = 0;

    
}

// Handler for the SIGUSR1 signal sent to scheduler upon successful termination of a process
void userHandler(int signum) {
    printf("Process successfully terminated.\n");
    succesful_exit_handler = 1;
}

// Calculates the standard deviation of the weighted turnaround times of all processes
void StandardDeviation(FILE* outCalcFile)
{
    // Average weighted turnaround time
    double AWTA = (double) (total_WTA/processCount);

    // Dequeuing from WTAQueue to calculate standard deviation
    double currDouble;
    double SD_numerator = 0;
    while (getQueueSize(&WTAQueue) != 0) {
        
        dequeue(&WTAQueue,&currDouble);
        double powerResult = pow((currDouble-AWTA), 2);
        SD_numerator += powerResult;
    }

    // Final result of the standard deviation of the weighted turnaround times
    double SD = sqrt(((double) (SD_numerator/processCount)));

    // Print the result in the output file
    fprintf(outCalcFile, "Std WTA = %.2f\n", SD);
}

// Shortest Remaining Time Next Algorithm
void SRTN(FILE* outLogFile,FILE* outMemFile) {

    printf("Running SRTN.\n");

    // Initialize a priority queue
    PNode* PQueueHead = NULL;
    struct msgbuff tempBuffer;
    PCB* tempPCB = NULL;
    int status;

    while(1) {

        ///////printf("Inside While\n");

        // In case of an empty priority queue
        if (isEmpty(&PQueueHead)) {

            // If the termination flag is 1; i.e. no more processes are coming
            if(finish_scheduler) {
                printf("Empty priority queue (%d) and finish flag is true\n", isEmpty(&PQueueHead));
                break;
            }

            // Wait till you receive a message 
            status = 0;
            while(!status) {
                printf("Will wait for message\n");
                tempBuffer = receiveMsg(1, &status);
            }

            if(status) {
                // Received message
                printf("Received process with id %d and pid %d\n", tempBuffer.data.id, tempBuffer.data.pid);

                // Check if incoming process is a flag
                if (tempBuffer.data.pid == -10)
                {
                    // Break
                    printf("Empty priority queue (%d) and received flag\n", isEmpty(&PQueueHead));
                    break;
                }
                else {
                    // If process is not flag
                    tempPCB = (PCB *) malloc(sizeof(PCB));  
                    equate(&tempBuffer.data, tempPCB); 
                    // Allocate memory to the new process
                    allocate_mem(tempPCB,outMemFile);                    
                    // Push process to the queue
                    push(&PQueueHead, tempPCB, tempPCB->remainingTime);
                    printf("Pushed process with id %d and pid %d\n", tempPCB->id, tempPCB->pid);
                }
            }
            else {
                printf("Empty priority queue and could not receive messsage\n");
            }           
        }

        status = 0;
        tempBuffer = receiveMsg(0, &status);
        while(status == 1) {
            
            printf("Received process with id %d and pid %d\n", tempBuffer.data.id, tempBuffer.data.pid);

            // Check for flag process
            if (tempBuffer.data.pid == -10)
            {
                finish_scheduler = 1;
                status = 0;
            }       
            else {
                // Not flag process
                tempPCB = (PCB *) malloc(sizeof(PCB));  
                equate(&tempBuffer.data, tempPCB); 
                // Allocate memory to the new process
                allocate_mem(tempPCB,outMemFile);                    
                // Push process to the queue
                push(&PQueueHead, tempPCB, tempPCB->remainingTime);
                printf("Pushed process with id %d and pid %d\n", tempPCB->id, tempPCB->pid);
                status = 0;
                tempBuffer = receiveMsg(0, &status);
            }
        }

        PCB* currProcessPCB = (PCB *) malloc(sizeof(PCB));      
        int afterPop = pop(&PQueueHead, currProcessPCB);
        
        printf("Successful popping (%d). Popped process with id %d and pid %d\n", afterPop, currProcessPCB->id, currProcessPCB->pid);

        // Chech whether process was forked before
        if (currProcessPCB->pid == -5) {
            // Start process
            startProcess(currProcessPCB, outLogFile);
            printf("Started process with id %d and pid %d\n", currProcessPCB->id, currProcessPCB->pid);
        }
        else {
            // Resume process
            resumeProcess(currProcessPCB, outLogFile, 0);
        }

        succesful_exit_handler = 0;

        // If termination flag is not set yet; i.e. more processes are still coming        
        if(!finish_scheduler) {
            printf("Will wait for message\n");
            status = 0;
            tempBuffer = receiveMsg(1, &status);
        }
        else {
            // No more processes are coming
            // Wait for current process to terminate
            int stat;
            pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
            if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
                printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
                if (WIFSIGNALED(stat)) {
                    psignal(WTERMSIG(stat), "Exit signal:");  
                }       
            }
            ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
            ///////printf("After sleep\n");
        }

        // If running process exits successfully
        if (succesful_exit_handler) {

            printf("Will finish process with id %d\n", currProcessPCB->id);
            finishProcess(currProcessPCB, outLogFile,outMemFile);
        }
        if (status == 1) {

            // A message was received
            // Check for flag process
            if (tempBuffer.data.pid != -10) {
                // Stop running process
                stopProcess(currProcessPCB, outLogFile, 0);
                succesful_exit_handler = 0;
                tempPCB = (PCB *) malloc(sizeof(PCB));  
                equate(&tempBuffer.data, tempPCB);   
                // Allocate memory to the new process
                allocate_mem(tempPCB,outMemFile);                     
                // Push both processes into the priority queue
                push(&PQueueHead, currProcessPCB, currProcessPCB->remainingTime);
                push(&PQueueHead, tempPCB, tempPCB->remainingTime);
                printf("Pushed process with id %d and pid %d\n", tempPCB->id, tempPCB->pid);
            }
            else {
                // Stop and resume running process just to correctly calculate all its attributes
                stopProcess(currProcessPCB, outLogFile, 0);
                succesful_exit_handler = 0;
                resumeProcess(currProcessPCB, outLogFile, 0);
                printf("Will wait for process with pid %d to finish\n", currProcessPCB->pid);

                // Wait for process to finish
                int stat;
                pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
                if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
                    printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
                    if (WIFSIGNALED(stat)) {
                        psignal(WTERMSIG(stat), "Exit signal:");  
                    }       
                }
                ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
                ///////printf("After sleep\n");

                finishProcess(currProcessPCB, outLogFile,outMemFile);
                printf("Finished process with id %d\n", currProcessPCB->id);

                // Set termination flag
                finish_scheduler = 1;
            }
        }
    }
    return;
}

void RR(FILE* outLogFile,FILE* outMemFile, int Quantum) {

    printf("Running RR\n");

    // Initializing ready queue
    Queue readyQueue;
    queueInit(&readyQueue, sizeof(PCB));
    struct msgbuff tempBuffer;
    PCB* tempPCB = NULL;
    int status;
    finish_scheduler = 0;

    while(1) {
        ///////printf("Inside While\n");

        // If queue is empty 
        if (getQueueSize(&readyQueue) == 0) {


            // If the termination flag is 1; i.e. no more processes are coming
            if(finish_scheduler) {
                printf("Empty queue (%d) and finish flag is true\n", getQueueSize(&readyQueue));
                break;
            }

            // Wait till you receive a message
            status = 0;
            while(!status) {
                printf("Will wait for message\n");
                tempBuffer = receiveMsg(1, &status);
            }

            if(status) {
                // Received message   
                printf("Received process with id %d and pid %d\n", tempBuffer.data.id, tempBuffer.data.pid);
                
                // Check if incoming process is a flag
                if (tempBuffer.data.pid == -10) {
                    printf("Empty queue (%d) and received flag\n", getQueueSize(&readyQueue));
                    break;
                }
                else {
                    // If process is not flag
                    tempPCB = (PCB *) malloc(sizeof(PCB));  
                    equate(&tempBuffer.data, tempPCB);
                    // Allocate memory to the new process
                    allocate_mem(tempPCB,outMemFile);                    
                    // Push process to the queue
                    enqueue(&readyQueue, tempPCB);
                    printf("Enqueued process with id %d and pid %d\n", tempPCB->id, tempPCB->pid);
                }
            }
            else {
                printf("Empty queue and could not receive messsage\n");
            }
        }

        status = 0;
        tempBuffer = receiveMsg(0, &status);
        while(status == 1) {

            printf("Received process with pid %d\n", tempBuffer.data.pid);   

            // Check for flag process
            if (tempBuffer.data.pid == -10)
            {
                finish_scheduler = 1;
                status = 0;
            }
            else {
                // Not flag process
                tempPCB = (PCB *) malloc(sizeof(PCB));  
                equate(&tempBuffer.data, tempPCB); 
                // Allocate memory to the new process
                allocate_mem(tempPCB,outMemFile);                    
                // Push process to the queue
                enqueue(&readyQueue, tempPCB);
                printf("Enqueued process with id %d\n", tempPCB->id);
                status = 0;
                tempBuffer = receiveMsg(0, &status);
            }
        }

        PCB* currProcessPCB = (PCB *) malloc(sizeof(PCB));      
        int afterDeq = dequeue(&readyQueue, currProcessPCB);
        
        printf("Successful dequeuing (%d) Dequeued process with id %d and pid %d\n", afterDeq, currProcessPCB->id, currProcessPCB->pid);
   
        // Chech whether process was forked before
        if (currProcessPCB->pid == -5) {
            // Start process
            startProcess(currProcessPCB, outLogFile);
            printf("Started process with id and %d pid %d\n", currProcessPCB->id, currProcessPCB->pid);
        }
        else {
            // Resume process
            resumeProcess(currProcessPCB, outLogFile, 0);
        }


        // Check if remaining time of process fits wihtin a quantum
        if (currProcessPCB->remainingTime <= Quantum) {
            
            // Wait for current process to terminate
            int stat;
            pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
            if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
                printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
                if (WIFSIGNALED(stat)) {
                    psignal(WTERMSIG(stat), "Exit signal:");  
                }       
            }
            ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
            ///////printf("After sleep\n");


            // Extra check that process exited successfully
            while (!succesful_exit_handler)
            {
                stopProcess(currProcessPCB, outLogFile, 1);
                resumeProcess(currProcessPCB, outLogFile, 1);
                int stat;
                pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
                if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
                    printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
                    if (WIFSIGNALED(stat)) {
                        psignal(WTERMSIG(stat), "Exit signal:");  
                    }       
                }
                ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
                ///////printf("After sleep\n");
            }

            finishProcess(currProcessPCB, outLogFile,outMemFile);
        }
        else {

            // If process remaining time does not fit in a quantum
            sleep(Quantum);

            // After quantum
            printf("Stopping process with id %d\n", currProcessPCB->id);
            stopProcess(currProcessPCB, outLogFile, 0);

            // Receive any process in the message queue
            status = 0;
            tempBuffer = receiveMsg(0, &status);
            while(status == 1) {
                printf("Received process with pid %d\n", tempBuffer.data.pid);   

                // Check for flag process
                if (tempBuffer.data.pid == -10)
                {
                    finish_scheduler = 1;
                    status = 0;
                }
                else {
                    tempPCB = (PCB *) malloc(sizeof(PCB));  
                    equate(&tempBuffer.data, tempPCB); 
                    // Allocate memory to the new process
                    allocate_mem(tempPCB,outMemFile);                    
                    enqueue(&readyQueue, tempPCB);
                    printf("Enqueued process with id %d", tempPCB->id);
                    status = 0;
                    tempBuffer = receiveMsg(0, &status);
                }
            }
            enqueue(&readyQueue, currProcessPCB);
        }  
    }
    return;
}

void HPF(FILE* outLogFile, FILE* outMemFile) {

    printf("Running HPF\n");

    // Initializing ready queue
    PNode* ReadyQueue = NULL;
    struct msgbuff tempBuffer;
    PCB* temp_process_pcb = (PCB *) malloc(sizeof(PCB));
    int status;
    finish_scheduler = 0;

    while(1) {

        ///////printf("Inside While\n");
        if (isEmpty(&ReadyQueue)) {

            // If the termination flag is 1; i.e. no more processes are coming
            if(finish_scheduler) {
                printf("Empty priority queue (%d) and finish flag is true\n", isEmpty(&ReadyQueue));
                break;
            }

            // Wait till you receive a message 
            status = 0;
            while(!status) {
                printf("Will wait for message\n");
                tempBuffer = receiveMsg(1, &status);
            }

            if(status) {
                // Received message  
                printf("Received process with id %d and pid %d\n", tempBuffer.data.id, tempBuffer.data.pid);
                
                // Check if incoming process is a flag
                if (tempBuffer.data.pid == -10)
                {
                    printf("Empty priority queue (%d) and received flag\n", isEmpty(&ReadyQueue));
                    break;
                }
                else {

                    // If process is not flag
                    temp_process_pcb = (PCB *) malloc(sizeof(PCB)); 
                    equate(&tempBuffer.data, temp_process_pcb); 
                    // Allocate memory to the new process
                    allocate_mem(temp_process_pcb,outMemFile);                    
                    push(&ReadyQueue, temp_process_pcb, temp_process_pcb->priority);
                    // Push process to the priority queue
                    printf("Pushed process with id %d and pid %d\n", temp_process_pcb->id, temp_process_pcb->pid);
                }
            }
            else {
                printf("Empty priority queue and could not receive messsage\n");
            }
        }

        status = 0;
        tempBuffer = receiveMsg(0, &status);
        while(status == 1) {
            printf("Received process with pid %d\n", tempBuffer.data.pid);   

            // Check for flag process
            if (tempBuffer.data.pid == -10)
            {
                finish_scheduler = 1;
                status = 0;
            }
            else {
                // Not flag process
                temp_process_pcb = (PCB *) malloc(sizeof(PCB)); 
                equate(&tempBuffer.data, temp_process_pcb); 
                // Allocate memory to the new process
                allocate_mem(temp_process_pcb,outMemFile); 
                // Push process to the priority queue
                push(&ReadyQueue, temp_process_pcb, temp_process_pcb->priority);
                printf("Pushed process with id %d and pid %d\n", temp_process_pcb->id, temp_process_pcb->pid);
                status = 0;
                tempBuffer = receiveMsg(0, &status);
            }
        }

        PCB* currProcessPCB = (PCB *) malloc(sizeof(PCB));      
        int afterPop = pop(&ReadyQueue, currProcessPCB);
        
        printf("Successful popping (%d). Popped process with id %d and pid %d\n", afterPop, currProcessPCB->id, currProcessPCB->pid);
        if (currProcessPCB->pid == -5) {
            startProcess(currProcessPCB, outLogFile);
            printf("Started id %d pid %d\n", currProcessPCB->id, currProcessPCB->pid);
        }
        
        // Wait for current process to terminate
        int stat;
        pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
        if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
            printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
            if (WIFSIGNALED(stat)) {
                psignal(WTERMSIG(stat), "Exit signal:");  
            }       
        }
        ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
        ///////printf("After sleep\n");


        // Extra check on successful termination of the process
        while (!succesful_exit_handler)
        {
            stopProcess(currProcessPCB, outLogFile, 1);
            resumeProcess(currProcessPCB, outLogFile, 1);
            int stat;
            pid_t isCurrProccess = waitpid(currProcessPCB->pid, &stat, 0);
            if ((isCurrProccess != currProcessPCB->pid) || !(WIFEXITED(stat))) {
                printf("Exit code received from process: %d.\n", WEXITSTATUS(stat));
                if (WIFSIGNALED(stat)) {
                    psignal(WTERMSIG(stat), "Exit signal:");  
                }       
            }
            ///////printf("Exit status from process %d.\n", WIFEXITED(stat));
            ///////printf("After sleep\n");
        }
        finishProcess(currProcessPCB, outLogFile,outMemFile);
        

        status = 0;
        tempBuffer = receiveMsg(0, &status);
        while(status == 1) {
            printf("Received process with pid %d\n", tempBuffer.data.pid); 

            // Check for flag process  
            if (tempBuffer.data.pid == -10)
            {
                finish_scheduler = 1;
                status = 0;
            }
            else {
                // Not flag
                temp_process_pcb = (PCB *) malloc(sizeof(PCB)); 
                equate(&tempBuffer.data, temp_process_pcb); 
                // Allocate memory to the new process
                allocate_mem(temp_process_pcb,outMemFile); 
                push(&ReadyQueue, temp_process_pcb, temp_process_pcb->priority);
                // Push process to the priority queue
                printf("Pushed process with id %d and pid %d\n", temp_process_pcb->id, temp_process_pcb->pid);
                status = 0;
                tempBuffer = receiveMsg(0, &status);
            }
        }
    }
    return;
}
//printing free chuncks lists
void DisplayChunkLists() {
    int chunck = 8;
    for (int i = 0; i < 8; i++) {
        printf("List Number %d: and it's chunck is: %d \n", i, chunck);
        chunck = chunck*2;
        display(&freeChunks[i]);
        printf("\n");

    }
}