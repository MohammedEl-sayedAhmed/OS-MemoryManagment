#include "headers.h"
#include <math.h>
#include <signal.h>
#include "linkedList.h"

LinkedList freeChunks[8];

int getFirstFreeChunk (int list_num) {
    
    if ((list_num < 0) || (list_num > 7)) {
        printf("Invalid required chunk size; out of boundaries.\n");
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

void DisplayChunkLists() {
    for (int i = 0; i < 8; i++) {
        printf("List Number %d: \n", i);
        display(&freeChunks[i]);
        printf("\n");
    }
}


int main() {

    //LinkedList freeChunks[8];

    for (int i = 0; i < 8; i++) {
        LLInit(&(freeChunks[i]), sizeof(int));
        printf("%d\n",freeChunks[i].sizeOfLL);
    }

    SortedInsert(&(freeChunks[7]), 1);
    printf("%d\n",freeChunks[7].sizeOfLL);

    DisplayChunkLists();


    //int mem_size = 255;

    //int mem_chunk_size = pow(2, ceil(log2(mem_size)));

    //int list_num = ceil(log2(mem_size)) - 3;

    //printf("%d\n", list_num);

    //int mem_size[10] = {8, 20, 127, 60, 200, 9, 16, 16, 16, 16};

    int mem_size[3] = {8, 20, 127};


    for (int m = 0; m < 3; m++) {

        int list_num = ceil(log2(mem_size[m])) - 3;
        int n = getFirstFreeChunk(list_num);

        printf("Allocated chunck number: %d\n\n", n);

        DisplayChunkLists();
    }

    return 0;
}
