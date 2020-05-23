#include <stdio.h>
#include <stdlib.h>
#include <time.h>
//#define null 0

#include "linkedList.h"


int main(int argc, char * argv[])
{
    
    LinkedList l;

    LLInit(&l, sizeof(int));

    //create(0);
    display(&l);

    for (int i = 100; i > 10; i=i-10){
        SortedInsert(&l, i);
        display(&l);
    }
    display(&l);
    printf("%d\n", l.sizeOfLL);
    SortedInsert(&l,55);
    SortedInsert(&l,9);
    SortedInsert(&l,100);
    SortedInsert(&l,40);
    SortedInsert(&l,56);

    display(&l);

    printf("%d\n", l.sizeOfLL);

    for (int i = l.sizeOfLL; i > 0; i--) {
        int f = delete_begin(&l);
        printf("%d\n", f);
    }
    display(&l);
}
