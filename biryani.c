#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#define ll long long
int chef[1000][2],n,table[1000],tslots[1000],total;
int students_time[1000],num_std,student[1000][2];
int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}
pthread_mutex_t tablelock[1000],stdlock[1000];
void biryani_ready(int k);
void ready_to_serve_table(int w);
void student_in_slot(int g);

void* fill_chefs_vessel(void* arg)
{
    int* k = (int*) arg;
    int q=*k;
    printf("chef %d started preparing food\n",q);    
    int w = 2+rand()%4;
    int r = 1+rand()%10;
    int p = 25+rand()%26;
    printf("Robot Chef %d is preparing %d vessels of Biryani\n",*k,r);
    sleep(w);
    printf("Robot Chef %d has prepared %d vessels of Biryani. Waiting for all the vessels to be emptied to resume cooking\n",*k,r);
    printf("chef %d completed preparing food\n",q);
    chef[*k][0] = r;
    chef[*k][1] = p;
    biryani_ready(*k);
    return NULL;
}
void biryani_ready(int k)
{
    while(chef[k][0]>0)
    {
	    for(int i=1;i<=n;i++)
	    {
	    	pthread_mutex_lock(&tablelock[i]);
	    	if(chef[k][0]>0)
		    {	
		    	if(table[i]==0)
		    	{
		            table[i]=chef[k][1];
		            chef[k][0]--;
		            printf("Robot Chef %d is refilling Serving Container of Serving Table %d\n",k,i);
		            printf("Serving table %d entering Serving Phase\n",i);
		            printf("Serving Container of Table %d is refilled by Robot Chef %d with capacity %d; Table %d resuming serving now\n",i,k,table[i],i);
		    	}
		    }
	    	pthread_mutex_unlock(&tablelock[i]);
	    	if(chef[k][0]==0)
	    		break;
	    }
	}
	printf("All the vessels prepared by Robot Chef %d are emptied. Resuming cooking now.\n",k);
	fill_chefs_vessel(&k);	
    
}
void* table_slots(void* arg)
{
	int *k = (int*) arg;
	while(1)
	{
		int f=0;
		while(table[*k]!=0)
		{
			if(tslots[*k]==0)
			{
				tslots[*k] = 1 + rand()%10;
				if(tslots[*k]>table[*k])
				    tslots[*k]=table[*k];
				printf("Serving Table %d is ready to serve with %d slots\n",*k,tslots[*k]);
				table[*k]-=tslots[*k];
				ready_to_serve_table(*k);
			}
			f=1; 
		}
	    if(f==1)
	    printf("Serving Container of Table %d is empty, waiting for refill",*k); 
	}
}
void ready_to_serve_table(int w)
{
		while(tslots[w]>0)
		{
		    for(int i=1;i<=num_std;i++)
		    {
		    	pthread_mutex_lock(&stdlock[i]);
		    	if(student[i][0]==1)
		    	{
	                if(tslots[w]!=0)
	                	{
	                		printf("Student %d assigned a slot on the serving table %d and waiting to be served\n",i,w);
	                		
	                		student[i][0]=2;
	                		student[i][1]=w;
	                		student_in_slot(i);
	                		tslots[w]--;
	                	}
		    	}
		    	pthread_mutex_unlock(&stdlock[i]);
		    	if(tslots[w]==0)
		    		break;
		    }
		}
	
}

void* wait_for_slot(void* arg)
{
    int *k = (int*) arg;
    student[*k][0]=1;
    printf("Student %d is waiting to be allocated a slot on the serving table\n",*k);
}

void student_in_slot(int g)
{    
    printf("Student %d on serving table %d has been served.\n",g,student[g][1]);
    student[g][0]=3;
}

int main()
{
    srand(time(0));
    int m,p,k;
    printf("Enter number of Chefs: ");
    scanf("%d",&m);
    printf("Enter number of Tables:");
    scanf("%d",&n);
    printf("Enter number of Students:");
    scanf("%d",&num_std);
    for(int i=0;i<num_std;i++)
    {
    	students_time[i] = rand()%60;
    }
    qsort (students_time, num_std, sizeof(int), comp);
    for(int i=0;i<num_std;i++)
    {
    	printf("%d ",students_time[i]);
    }
    printf("\n");
    //Creating mutex locks for each table so that a table is filled by 2 vessels simultaneously
    for(int i=1;i<=n;i++)
    {
        if(pthread_mutex_init(&tablelock[i],NULL)!=0)
        {
        	printf("Unable to create mutex lock.\nProgram failed.\n");
        	exit(1);
        }
    }
    //Creating mutex locks for each student so that a student is not served by two tables simultaneously
    for(int i=1;i<=num_std;i++)
    {
        if(pthread_mutex_init(&stdlock[i],NULL)!=0)
        {
        	printf("Unable to create mutex lock.\nProgram failed.\n");
        	exit(1);
        }
    }
    pthread_t tid;
    int qw[1000];
    for(int i=1;i<=m;i++)
    {
        qw[i]=i;
        pthread_create(&tid, NULL, &fill_chefs_vessel, &qw[i]);
    }
    int er[1000];
    for(int i=1;i<=n;i++)
    {
        er[i]=i;
        pthread_create(&tid, NULL, &table_slots, &er[i]);
    }
    int s=0;
    int we[1000];
    for(int i=0;i<num_std;i++)
    {
        sleep(students_time[i]-s);
        s=students_time[i];
        we[i]=i+1;
        printf("Student %d has arrived\n",i+1);
        pthread_create(&tid, NULL, &wait_for_slot, &we[i]);
    }
    while(1)
    {
    	int asd=0;
    	for(int i=1;i<=num_std;i++)
    	{
    		if(student[i][0]==3)
    			asd++;
    	}
    	if(asd==num_std)
    	{
    		printf("Simulation over!!!\n");
    		exit(1);
    	}
        sleep(2);
    } 
}