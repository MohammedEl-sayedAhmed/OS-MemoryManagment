#include<stdlib.h>
#include <stdio.h>
//#include <stdbool.h>

 
struct node
{
	int info;
	struct node *next;
};

typedef struct LinkedList
{
    int sizeOfLL;
    size_t memSize;
    struct node *start;
}LinkedList;

//struct node *start=NULL;

void LLInit(LinkedList *l, size_t memSize) {
	l->sizeOfLL = 0;
    l->memSize = memSize;
    l->start = NULL;
}

/*
void create(int myInfo)
{

	struct node *temp,*ptr;
	temp=(struct node *)malloc(sizeof(struct node));
	if(temp==NULL)
	{
		printf("Out of Memory Space:\n");
		exit(0);
	}
	//printf("Enter the data value for the node:\t");
	temp->info = myInfo;
	//scanf("%d",&temp->info);
	temp->next=NULL;
	if(start==NULL)
	{
		start=temp;
	}
	else
	{
		ptr=start;
		while(ptr->next!=NULL)
		{
			ptr=ptr->next;
		}
		ptr->next=temp;
	}
}
*/

void display(LinkedList *l)
{
	struct node *ptr;
	if(l->start==NULL)
	{
		printf("List is empty.\n");
		return;
	}
	else
	{
		ptr = l->start;
		printf("List Elements:\t\t");
		while(ptr!=NULL)
		{
			printf("%d\t",ptr->info );
			ptr=ptr->next ;
		}
	}
	printf("\n");
}

void insert_begin(LinkedList *l, int myInfo)
{
	struct node *temp;
	temp=(struct node *)malloc(sizeof(struct node));
	if(temp==NULL)
	{
		printf("\nOut of Memory Space");
		return;
	}
	//printf("\nEnter the data value for the node:\t" );
	temp->info = myInfo;
	//scanf("%d",&temp->info);
	temp->next =NULL;
	if(l->start==NULL)
	{
		l->start=temp;
	}
	else
	{
		temp->next=l->start;
		l->start=temp;
	}

	(l->sizeOfLL)++;
}

void insert_end(LinkedList *l, int myInfo)
{
	struct node *temp,*ptr;
	temp=(struct node *)malloc(sizeof(struct node));
	if(temp==NULL)
	{
		printf("\nOut of Memory Space");
		return;
	}
	//printf("\nEnter the data value for the node:\t" );
	temp->info = myInfo;
	//scanf("%d",&temp->info );
	temp->next =NULL;
	if(l->start==NULL)
	{
		l->start=temp;
	}
	else
	{
		ptr = l->start;
		while(ptr->next !=NULL)
		{
			ptr=ptr->next ;
		}
		ptr->next =temp;
	}

	(l->sizeOfLL)++;
}

void insert_pos(LinkedList *l, int myPos, int myInfo)
{
	struct node *ptr,*temp;
	int i,pos;
	temp=(struct node *)malloc(sizeof(struct node));
	if(temp==NULL)
	{
		printf("\nOut of Memory Space");
		return;
	}
	//printf("\nEnter the position for the new node to be inserted:\t");
	pos = myPos;
	//scanf("%d",&pos);
	//printf("\nEnter the data value of the node:\t");
	temp->info = myInfo;
	//scanf("%d",&temp->info) ;
  
	temp->next=NULL;
	if(pos==0)
	{
		temp->next=l->start;
		l->start=temp;
	}
	else
	{
		for(i=0,ptr=l->start;i<pos-1;i++) { ptr=ptr->next;
			if(ptr==NULL)
			{
				printf("\nPosition not found:[Handle with care]");
				return;
			}
		}
		temp->next =ptr->next ;
		ptr->next=temp;
	}

	(l->sizeOfLL)++;
}

int delete_begin(LinkedList *l)
{
	struct node *ptr;
	if(l->start==NULL)
	{
		printf("List is empty.\n\n");
		return -1;
	}
	else
	{
		ptr = l->start;
		l->start = (l->start)->next;

		int delInt = ptr->info;
		//printf("\nThe deleted element is :%d\n", delInt);
		free(ptr);

		(l->sizeOfLL)--;

		return delInt;
	}
}

void delete_end(LinkedList *l)
{
	struct node *temp,*ptr;
	if(l->start==NULL)
	{
		printf("\nList is Empty");
		return;
	}
	else if(l->start->next ==NULL)
	{
		ptr=l->start;
		l->start=NULL;
		printf("\nThe deleted element is:%d\t",ptr->info);
		free(ptr);
	}
	else
	{
		ptr=l->start;
		while(ptr->next!=NULL)
		{
			temp=ptr;
			ptr=ptr->next;
		}
		temp->next=NULL;
		printf("\nThe deleted element is:%d\t",ptr->info);
		free(ptr);
	}

	(l->sizeOfLL)--;
}

void delete_pos(LinkedList *l, int myPos)
{
	int i,pos;
	struct node *temp,*ptr;
	if(l->start==NULL)
	{
		printf("\nThe List is Empty");
		return;
	}
	else
	{
		//printf("\nEnter the position of the node to be deleted:\t");
		pos = myPos;
		//scanf("%d",&pos);
		if(pos==0)
		{
			ptr=l->start;
			l->start=(l->start)->next ;
			printf("\nThe deleted element is:%d\t",ptr->info  );
			free(ptr);
		}
		else
		{
			ptr=l->start;
			for(i=0;i<pos;i++) { temp=ptr; ptr=ptr->next ;
				if(ptr==NULL)
				{
					printf("\nPosition not Found");
					return;
				}
			}
			temp->next =ptr->next ;
			printf("\nThe deleted element is:%d\t",ptr->info );
			free(ptr);
		}
	}

	(l->sizeOfLL)--;
}

void SortedInsert(LinkedList *l, int x){


	struct node *p =l->start;
	struct node *t = NULL, *q = NULL;
	t = (struct node *)malloc(sizeof(struct node));
	t->info = x;
	t->next = NULL;

	if (l->start == NULL){
		l->start = t;
	}             
	else if (l->start->info > x) 
	{
		t->next = l->start;
		l->start = t;
	} 
	else
	{
		while (p && p->info < x){
			
			q = p;
			p = p->next;		
		}	
		t->next = q->next;
		q->next = t;    
	}

	(l->sizeOfLL)++;
}

int findPos(LinkedList *l, int value){
	struct node *p =l->start;
	
	//if the linkedlist is empty, return 0
	if (l->start == NULL){
		return 0;
	}
	else
	{
		//loop through the linked list as long as the element is not found
		while (p && p->info != value){
			p = p->next;		
		}	

		//check if the loop exited because the element is found, not because 
		//p is a null pointer
		if (p == NULL)
		{
			return 0;   //the while loop is exited due to the first condition
		}
		else
		{
			return 1; //the while loop is exited due to the second condition
		}	
	}
}

int delete_element(LinkedList *l, int value){	
	//if the linkedlist is empty, return 0
	if (l->start == NULL){
		printf("The list is empty, no element to be deleted");
		return 0;
	}

	struct node *p = l->start;
	struct node *q = p->next;
	
	//if the element is in the first node
	if (l->start->info == value)
	{
		l->start = p->next;
		(l->sizeOfLL)--;
		return 1;
	}

	//check if the element is in the proceeding nodes

	while(q){
		if(q->info == value){
			p->next = q->next;
			q = NULL;
			(l->sizeOfLL)--;
			return 1;
			}
		p = q;
		q = p->next;
		}
		return 0;
}
