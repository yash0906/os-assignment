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
#include<sys/time.h>
#include<semaphore.h>
#include<signal.h>
#include<assert.h>
#include<error.h>
pthread_mutex_t cab_mutex[1000],pay_mutex[1000];
pthread_t rider_tid[1000];
sem_t sem_cab,sem_pay;
int cab[1000],num_cabs,num_pay,num_riders,pay_server[1000];
typedef struct rider_data
{
	int max_wait;
	int ride_time;
	int cab_type;
	int num;
}rider_data;
void* payment(void* arg)
{
	int* k = (int*)arg;
	int l=-1;
	sem_wait(&sem_pay);
	for(int i=1;i<=num_pay;i++)
	{
		if(pay_server[i]==0)
		{	
			pthread_mutex_lock(&pay_mutex[i]);
			if(pay_server[i]==0)
			{ 
	            l=i;
	            pay_server[i]=1;
	            //printf("%d\n",i);
			}
			pthread_mutex_unlock(&pay_mutex[i]);
		}
		if(l!=-1)
			break;
	}
	pthread_mutex_lock(&pay_mutex[l]);
	sleep(2);
	pay_server[l]=0;
	pthread_mutex_unlock(&pay_mutex[l]);
	printf("Rider %d is done with payment and had made payment on server %d\n",*k,l);
    sem_post(&sem_pay);
	return NULL;
}
void* booking(void* arg)
{
    rider_data* rider = (rider_data*)arg;
    if(rider->cab_type==0)
    {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
		{
            printf("Some error occured for rider %d\n",rider->num);
            pthread_exit(NULL);
		}
		ts.tv_sec += rider->max_wait;
		int k = sem_timedwait(&sem_cab, &ts);
		if(k==-1)
		{
			printf("Rider %d has to wait longer then the maximum wait time\n",rider->num);
			pthread_exit(NULL);
		} 
		else
		{
			int l=-1;
			for(int i=1;i<=num_cabs;i++)
			{
				if(cab[i]==0)
				{
					pthread_mutex_lock(&cab_mutex[i]);
					if(cab[i]==0)
					{
	                    l=i;
	                    cab[i]=1;
					}
					pthread_mutex_unlock(&cab_mutex[i]);
				}
				if(l!=-1)
					break;
			}
			printf("Premier rider %d has found a cab with number %d \n",rider->num,l);
			sleep(rider->ride_time);
			cab[l]=0;
			sem_post(&sem_cab);
			printf("Rider %d has reached the destination and cab driver with number %d is now free to take other rides\n",rider->num, l);

		}
    }
    else
    {  
        struct timespec curr,end;
        if (clock_gettime(CLOCK_REALTIME, &curr) == -1)
		{
           	printf("Some error occured for rider %d\n",rider->num);
           	pthread_exit(NULL);
		}
		if (clock_gettime(CLOCK_REALTIME, &end) == -1)
		{
           	printf("Some error occured for rider %d\n",rider->num);
           	pthread_exit(NULL);
		}
		end.tv_sec += rider->max_wait;
		int f=0,t=-1,l=-1;

		while(curr.tv_sec<end.tv_sec)
		{
			for(int i=1;i<=num_cabs;i++)
			{
		        pthread_mutex_lock(&cab_mutex[i]);
            	if(cab[i]==2)
            	{
               		f=1;
               		l=i;
               		t=0;// this variable is for whether we have to change semaphore or not.
               		cab[i]=3;
            	}
            	if(cab[i]==0)
            	{
               		if(sem_trywait(&sem_cab)==0)
               		{
	               		f=1;
	               		l=i;
	               		t=1;// this variable is for whether we have to change semaphore or not.
	               		cab[i]=1;
	               	}

            	}
       			pthread_mutex_unlock(&cab_mutex[i]); 
       			if(f==1)
       				break;		
			}
			if(f==1)
				break;
			if (clock_gettime(CLOCK_REALTIME, &curr) == -1)
				{
           			printf("Some error occured for rider %d\n",rider->num);
           			pthread_exit(NULL);
				}
		}
        if(f==1)
        {
            if(t==0)
            {
	            printf("Rider %d found Pool cab with number %d ans it has 2 Passengers in it.\n",rider->num,l);
	            sleep(rider->ride_time);
	            printf("Rider %d has reached the destination and cab driver with number %d is now free to take other rides\n",rider->num, l);
	            pthread_mutex_lock(&cab_mutex[l]);
	            if(cab[l]==2)
	            	cab[l]=0;
	            if(cab[l]==3)
	            	cab[l]=2;
	            pthread_mutex_unlock(&cab_mutex[l]);
	        }
	        else
	        {
                printf("Poll rider %d has found a cab with number %d which has only one passenger in it.\n",rider->num,l);
				sleep(rider->ride_time);
				pthread_mutex_lock(&cab_mutex[l]);
				cab[l]=0;
				sem_post(&sem_cab);
				pthread_mutex_unlock(&cab_mutex[l]);
				
				printf("Pool Rider %d has reached the destination and cab driver %d is now free to take other rides\n",rider->num, l);

	        }

        }
        else
        {
        	printf("Rider %d has to wait longer then the maximum wait time\n",rider->num);
			pthread_exit(NULL);
        }  
    }
    
    pthread_t tid;
    int k=rider->num;
    pthread_create(&tid,NULL,&payment,&k);
    pthread_join(tid,NULL);
    
}
int main()
{
    srand(time(0));
    printf("Enter number of cabs: ");
    scanf("%d",&num_cabs);
    printf("Enter number of payment counter: ");
    scanf("%d",&num_pay);
    printf("Enter number of riders: ");
    scanf("%d",&num_riders);
    struct rider_data data[num_riders+2];
    for(int i=1;i<=num_cabs;i++)
    {
    	if(pthread_mutex_init(&cab_mutex[i],NULL)!=0)
    	{
    		printf("Unable to create mutex lock.\nProgram failed.\n");
        	exit(1);
    	}
    }
    for(int i=1;i<=num_pay;i++)
    {
    	if(pthread_mutex_init(&pay_mutex[i],NULL)!=0)
    	{
    		printf("Unable to create mutex lock.\nProgram failed.\n");
        	exit(1);
    	}
    }
    for(int i=1;i<=num_riders;i++)
    {
        data[i].max_wait = rand()%10;
        if(data[i].max_wait<5)
        	data[i].max_wait = 5;
        data[i].ride_time = rand()%10;
        if(data[i].ride_time<5)
        	data[i].ride_time = 5;
        //data[i].ride_time=0;
        data[i].cab_type = rand()%2;// 0 for premium and 1 for poll
        data[i].num = i;
    } 
    
    sem_init(&sem_cab, 0 , num_cabs);
    //printf("%d\n",num_pay);
    sem_init(&sem_pay, 0 , num_pay);
    for(int i=1;i<=num_riders;i++)
    {
    	printf("Rider %d is waiting for cab.\n",i);
    	pthread_create(&rider_tid[i], NULL, &booking, &data[i]);
    	sleep(1);
    }
    for(int i=1;i<=num_riders;i++)
    {
    	pthread_join(rider_tid[i],NULL);
    }
} 
