#include "headers.h"
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

void Allocation(PCB* processPCB, FILE* outLogFile) ;
void deAllocation(PCB* processPCB, FILE* outLogFile) ;
int main()
{
FILE* outLogFile = (FILE *) malloc(sizeof(FILE));
    outLogFile = fopen("memoryLog.txt", "w");
    if (outLogFile == NULL) {
        printf("Could not open output file for memory log.\n");
    }

       // Close the output log file
    fclose(outLogFile);
    

    return 0;
}
void Allocation(PCB* processPCB, FILE* outLogFile) {

    int currTime1 = getClk();
    fprintf(outLogFile, "At time %d allocated %d bytes for process %d from  %d to %d\n", currTime1, processPCB->bytes, processPCB->startingbyte, processPCB->endbyte);
    }

/////////////////////////////////////////
void Deallocation(PCB* processPCB, FILE* outLogFile) {
    int currTime2= getClk();

    fprintf(outLogFile, "At time %d freed %d bytes for process %d from  %d to %d\n", currTime2, processPCB->bytes, processPCB->startingbyte, processPCB->endbyte);
    }
