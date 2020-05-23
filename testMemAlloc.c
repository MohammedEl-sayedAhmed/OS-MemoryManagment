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

/*void DisplayChunkLists() {
    for (int i = 0; i < 8; i++) {
        printf("List Number %d: \n", i);
        display(&freeChunks[i]);
        printf("\n");
    }
}*/

void DisplayChunkLists() {
    int chunck = 8;
    for (int i = 0; i < 8; i++) {
        printf("List Number %d: and it's chunck is: %d \n", i, chunck);
        chunck = chunck*2;
        display(&freeChunks[i]);
        printf("\n");

    }
}

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


int main() {

    //LinkedList freeChunks[8];

    for (int i = 0; i < 8; i++) {
        LLInit(&(freeChunks[i]), sizeof(int));
        printf("%d\n",freeChunks[i].sizeOfLL);
    }

    SortedInsert(&(freeChunks[7]), 1);
    printf("%d\n",freeChunks[7].sizeOfLL);

    //DisplayChunkLists();


    //int mem_size = 255;

    //int mem_chunk_size = pow(2, ceil(log2(mem_size)));

    //int list_num = ceil(log2(mem_size)) - 3;

    //printf("%d\n", list_num);

    int mem_size[10] = {8, 20, 127, 60, 200, 9, 16, 16, 16, 16};

    //int mem_size[3] = {8, 20, 127};
    int alloc_mem[10];
    
    for (int m = 0; m < 10; m++) {

        int list_num = ceil(log2(mem_size[m])) - 3;
        int n = getFirstFreeChunk(list_num);

        printf("Allocated chunck number: %d\n\n", n);
        alloc_mem[m] = n;
        //DisplayChunkLists();
    }
    DisplayChunkLists();
    printf("\n [");
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", alloc_mem[i]);
    }
    printf("] \n");

    
    printf("\n\nDeallocate\n\n");

    for (int m = 0; m < 10; m++)
    {
        int list_num = ceil(log2(mem_size[m])) - 3;
        int del = freeChunck(list_num, alloc_mem[m]);
        printf("\nfreed chenck status is %d \n\n", del);
        mergeFreeChuncks(list_num, del);
        DisplayChunkLists();
    }

        
    return 0;

}
