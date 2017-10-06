#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>

void *car(void *arg);
void *attender(void *arg);
int enterStation(int, int);
int waitInLine(int);
void goToPump(int, int);
void pay(int);
void exitStation(int);
void serveCar(int);
void acceptPayment(int);

#define SHMSIZE 4
key_t key = IPC_PRIVATE; /* This is needed */

typedef struct passParamsCar {                                                   
    int time;                                                             
    int num;                                                              
}carInf;

typedef struct passParamsAttender {                                                   
    int num;                                                              
}attInf;

sem_t pump;
sem_t waiting;

pthread_mutex_t pumps[4];
pthread_mutex_t pay_w[4];
pthread_mutex_t pay_wait;
pthread_cond_t cond[4];
pthread_cond_t cond_p[4];

// int atts_arr[4];
// int pays_arr[4];
// int pumps_arr[4];
int *atts_arr;
int *pays_arr;
int *pumps_arr;
int num_of_cars; 

int main()
{

	int shmid, shmid1, shmid2, i, status;
	shmid = shmget(key, sizeof(int) *SHMSIZE, IPC_CREAT| 0666);
	if(shmid == -1) {
		perror("Shmget failed");
		exit(1);
	}
	shmid1 = shmget(key, sizeof(int) *SHMSIZE, IPC_CREAT| 0666);
	if(shmid1 == -1) {
		perror("Shmget failed");
		exit(1);
	}	
	shmid2 = shmget(key, sizeof(int) *SHMSIZE, IPC_CREAT| 0666);
	if(shmid2 == -1) {
		perror("Shmget failed");
		exit(1);
	}
	pumps_arr = shmat(shmid, 0, 0);
	if(pumps_arr == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}
	atts_arr = shmat(shmid1, 0, 0);
	if(atts_arr == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}	
	pays_arr = shmat(shmid2, 0, 0);
	if(pays_arr == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}	

	time_t t;
	srand((unsigned) time(&t));
	pthread_t tid;

    sem_init(&pump,0, 3);
    sem_init(&waiting,0, 4);

	int no;
	printf("Enter number of cars: ");
	scanf("%d", &no);
	num_of_cars = no;
	printf("Number of cars inputed - %d\n\n", no );

	for (i = 0; i < 4; ++i)
	{
    	pthread_cond_init(&cond[i], NULL);
    	pthread_cond_init(&cond_p[i], NULL);
    	pthread_mutex_init(&pumps[i], NULL);
    	pthread_mutex_init(&pay_w[i], NULL);
    	pumps_arr[i] = 0;
    	atts_arr[i] = 0;
    	pays_arr[i] = 0;
	}
	atts_arr[0] = 4;

	attInf *a_params[no];
	for (i = 0; i < no; ++i)
	{
		a_params[i] = malloc(sizeof(attInf));
		a_params[i]->num = i+1;
    	pthread_create(&tid, NULL, attender, a_params[i]);
	}		

	sleep(1);

	carInf *params[no];
    int time = 0;
	for (i = 0; i < no; ++i)
	{
		params[i] = malloc(sizeof(carInf));
		time = time + rand()%3;
		params[i]->time = time;
		params[i]->num = i+1;
		printf("%d\n", time);
    	pthread_create(&tid, NULL, car, params[i]);
	}	


    pthread_exit(NULL);

	return 0;
}

void *car(void* arg)
{
	carInf *params = arg;
	int num = params->num;
	int stime = params->time;
	int pumpno;

	int out = enterStation(num, stime);
	if(out == 1)
	{
		pumpno = waitInLine(num);
		goToPump(num, pumpno);
		// pay(num);
		exitStation(num);
	}

}

void *attender(void* arg)
{
	attInf *params = arg;
	int num = params->num;
	// printf("Start Attender %d\n", num );
	while(num_of_cars)
	{
		if(atts_arr[num] == 0 && pumps_arr[num] == 1)
		{
			// printf("Here\n");
			serveCar(num);
		}
		else if(atts_arr[num] == 2 && pays_arr[num] != 0)
		{
			acceptPayment(num);
		}
	}
	// printf("Done Attender %d\n", num );
}

void serveCar(int num)
{
	// Use sleep 1
	printf("Attender %d - is serving at pump %d \n", num, num);
	sleep(1);
    pthread_cond_signal(&cond[num]);
	atts_arr[num] = 0;
	printf("Attender %d - finished serving at pump %d \n", num, num);
}

void acceptPayment(int num)
{
	// Use sleep 0.1 - and through one ATM only
	printf("Attender %d - is accepting payment from car no %d\n", num, pays_arr[num]);
	sleep(0.5);
    pthread_cond_signal(&cond_p[num]);
	atts_arr[num] = 0;
}

int enterStation(int num, int stime)
{
	sleep(stime);
	int *cars = malloc(sizeof(int));
	sem_getvalue(&waiting, cars);
	int free_slots = *cars;

	if(free_slots == 0)
	{
		printf("Car %d - has left station - waiting line full\n", num);
		return 0;
	}
	else
	{
		printf("Car %d - has entered station, at time %d\n", num, stime);
    	return 1;
	}
}

int waitInLine(int num)
{
	int *pumps = malloc(sizeof(int));
	sem_getvalue(&pump, pumps);	
	int free_pumps = *pumps;
	int i;

	if(free_pumps == 0) // Then Wait until pump free
	{
    	sem_wait(&waiting);
		printf("Car %d - is waiting in line \n", num);
		sem_wait(&pump);
		for (i = 1; i < 4; ++i)
		{
			if(pumps_arr[i] == 0)
			{
				pumps_arr[i] = 1;
				break;
			}
		}
		sem_post(&waiting);
		return i;
	}
	else
	{
		printf("No waiting for car , a pump is free!\n");
		sem_wait(&pump);
		for (i = 1; i < 4; ++i)
		{
			if(pumps_arr[i] == 0)
			{
				pumps_arr[i] = 1;
				break;
			}
		}		
		return i;
	}
}

void goToPump(int num, int pumpnum)
{
    pthread_mutex_lock(&pumps[pumpnum]);
	printf("Car %d - is going to Pump no %d \n", num, pumpnum);
	// sleep(3);
	printf("Car %d - is waiting at Pump no %d \n", num, pumpnum);
    pthread_cond_wait(&cond[pumpnum], &pumps[pumpnum]);
	pumps_arr[pumpnum] = 0;
	sem_post(&pump);
    pthread_mutex_unlock(&pumps[pumpnum]);
}

void pay(int num)
{
	int i; // has attender number
	i = 0;
	printf("Car %d - is waiting to pay \n", num );
    pthread_mutex_lock(&pay_wait);
	while(atts_arr[i] != 2)
	{
		for (i = 1; i < 4; ++i)
		{
			if(atts_arr[i] == 0)
			{
				atts_arr[i] = 2;
				printf("Car %d - is allted pay to attender no %d \n", num, i);
				break;
			}		
		}
	}
	printf("Car %d - is paying to attender no %d \n", num, i);
	// sleep(1);
	pays_arr[i] = num;
    pthread_cond_wait(&cond_p[i], &pay_w[i]);
	pays_arr[i] = 0;
	num_of_cars--;
	if(num_of_cars == 0)
		printf("Last Car to leave\n");
    pthread_mutex_unlock(&pay_wait);

}

void exitStation(int num)
{
	num_of_cars--;
	if(num_of_cars == 0)
		printf("Last Car to leave\n");
	printf("Car %d - is exiting the station\n", num);
}