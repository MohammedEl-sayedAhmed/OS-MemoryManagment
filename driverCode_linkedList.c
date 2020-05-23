#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define null 0

#include "linkedList.h"


int main(int argc, char * argv[])
{
    
    //create(0);
    display();
    for (int i = 10; i < 100; i=i+10){
        insert_begin(i);

    }
    display();
}
