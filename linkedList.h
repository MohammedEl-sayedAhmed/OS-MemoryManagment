#include<stdlib.h>
#include <stdio.h>
//#include <stdbool.h>

     
 
 
struct node
{
	int info;
	struct node *next;
};
struct node *start=NULL;

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

void display()
{
	struct node *ptr;
	if(start==NULL)
	{
		printf("\nList is empty");
		return;
	}
	else
	{
		ptr=start;
		printf("\nThe List elements are:\t\t");
		while(ptr!=NULL)
		{
			printf("%d\t",ptr->info );
			ptr=ptr->next ;
		}
	}
	printf("\n");
}

void insert_begin(int myInfo)
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
	if(start==NULL)
	{
		start=temp;
	}
	else
	{
		temp->next=start;
		start=temp;
	}
}

void insert_end(int myInfo)
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
	if(start==NULL)
	{
		start=temp;
	}
	else
	{
		ptr=start;
		while(ptr->next !=NULL)
		{
			ptr=ptr->next ;
		}
		ptr->next =temp;
	}
}

void insert_pos(int myPos, int myInfo)
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
		temp->next=start;
		start=temp;
	}
	else
	{
		for(i=0,ptr=start;i<pos-1;i++) { ptr=ptr->next;
			if(ptr==NULL)
			{
				printf("\nPosition not found:[Handle with care]");
				return;
			}
		}
		temp->next =ptr->next ;
		ptr->next=temp;
	}
}

void delete_begin()
{
	struct node *ptr;
	if(ptr==NULL)
	{
		printf("\nList is Empty");
		return;
	}
	else
	{
		ptr=start;
		start=start->next ;
		printf("\nThe deleted element is :%d\t",ptr->info);
		free(ptr);
	}
}

void delete_end()
{
	struct node *temp,*ptr;
	if(start==NULL)
	{
		printf("\nList is Empty");
		exit(0);
	}
	else if(start->next ==NULL)
	{
		ptr=start;
		start=NULL;
		printf("\nThe deleted element is:%d\t",ptr->info);
		free(ptr);
	}
	else
	{
		ptr=start;
		while(ptr->next!=NULL)
		{
			temp=ptr;
			ptr=ptr->next;
		}
		temp->next=NULL;
		printf("\nThe deleted element is:%d\t",ptr->info);
		free(ptr);
	}
}

void delete_pos(int myPos)
{
	int i,pos;
	struct node *temp,*ptr;
	if(start==NULL)
	{
		printf("\nThe List is Empty");
		exit(0);
	}
	else
	{
		//printf("\nEnter the position of the node to be deleted:\t");
		pos = myPos;
		//scanf("%d",&pos);
		if(pos==0)
		{
			ptr=start;
			start=start->next ;
			printf("\nThe deleted element is:%d\t",ptr->info  );
			free(ptr);
		}
		else
		{
			ptr=start;
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
}

void SortedInsert(int x){


	struct node *p =start;
	struct node *t = NULL, *q = NULL;
	t = (struct node *)malloc(sizeof(struct node));
	t->info = x;
	t->next = NULL;

	if (start == NULL){
		start = t;
	}             
	else if (start->info > x) 
	{
		t->next = start;
		start = t;
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
}