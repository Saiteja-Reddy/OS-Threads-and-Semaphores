#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>

#define SHMSIZE 4
key_t key = IPC_PRIVATE; /* This is needed */

sem_t pump;
sem_t waiting;

pthread_mutex_t mutex[4];
pthread_mutex_t mutex_p[4];
pthread_cond_t cond[4];
pthread_cond_t cond_p[4];

void *car(void *arg);
void *attender(void *arg);
int enterStation(int, int);
int waitInLine(int);
void goToPump(int, int);
void pay(int);
void exitStation(int);
void serveCar(int);
void acceptPayment(int);

struct passParamsCar {                                                   
    int time;                                                             
    int num;                                                              
};

struct passParamsAttender {                                                   
    int num;                                                              
};

int *pumps_array;
int *atts_array;
int *pays_array;
int *waiting_array;

int main()
{

	int shmid, shmid1, shmid2, shmid3, i,status;
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
	shmid3 = shmget(key, sizeof(int) *SHMSIZE, IPC_CREAT| 0666);
	if(shmid3 == -1) {
		perror("Shmget failed");
		exit(1);
	}	
	pumps_array = shmat(shmid, 0, 0);
	if(pumps_array == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}
	atts_array = shmat(shmid1, 0, 0);
	if(atts_array == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}	
	pays_array = shmat(shmid2, 0, 0);
	if(pays_array == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}		
	waiting_array = shmat(shmid3, 0, 0);
	if(waiting_array == (void *)-1) {
		perror("Shmat failed");
		exit(1);
	}			
	for(i = 0; i < SHMSIZE; i++) {
		pumps_array[i] = 0;
		atts_array[i] = 0;
		pays_array[i] = 0;
		waiting_array[i] = 0;
    	pthread_mutex_init(&mutex[i], NULL);
    	pthread_mutex_init(&mutex_p[i], NULL);
    	pthread_cond_init(&cond[i], NULL);		
    	pthread_cond_init(&cond_p[i], NULL);		
	}	

	int no;
	printf("Enter number of cars: ");
	scanf("%d", &no);
	printf("Number of cars inputed - %d\n\n", no );
    
	time_t t;
	srand((unsigned) time(&t));
	
	pthread_t tid[no];

	pthread_t a_tid[3];

	struct passParamsAttender *a_params[3];
	for (i = 0; i < 3; ++i)
	{
		a_params[i] = malloc(sizeof(struct passParamsAttender));
		a_params[i]->num = i+1;
		// printf("%d\n", a_params[i]->num );
    	pthread_create(&a_tid[i], NULL, attender, a_params[i]);
	}


	struct passParamsCar *params[no];

    sem_init(&pump,0, 3);
    sem_init(&waiting,0, 4);


    int time = 0;
	for (i = 0; i < no; ++i)
	{
		params[i] = malloc(sizeof(struct passParamsCar));
		time = time + rand()%3;
		params[i]->time = time;
		params[i]->num = i+1;
		printf("%d\n", time);
		//printf("%d %d\n", params[i]->num, params[i]->time );
    	pthread_create(&tid[i], NULL, car, params[i]);
	}

	for (i = 0; i < no; ++i)
	{
    	pthread_join(tid[i], NULL);
	}
	for (i = 0; i < 3; ++i)
	{
    	pthread_join(a_tid[i], NULL);
	}

	return 0;
}

void *car(void *arg)
{
	struct passParamsCar *params = arg;
	int num = params->num;
	int stime = params->time;


	int out = enterStation(num, stime);
	if(out == 0)
		exitStation(num);
	else
	{
		int pnum = waitInLine(num);
		goToPump(num, pnum);
		pay(num);
		exitStation(num);

	}
	printf("\n");
}

void *attender(void* arg)
{
	struct passParamsAttender *params = arg;
	int num = params->num;

	while(1)
	{
		if(pumps_array[num] == 1 && atts_array[num] == 0)
		{
			serveCar(num);
		}
		if(pays_array[num] != 0 && atts_array[num] == 2)
		{
			acceptPayment(num);
		}
	}
}

void serveCar(int num)
{
	// Use sleep 1
	atts_array[num] = 1;
	printf("Attender %d - is serving at pump - \n", num);
	sleep(3);
    pthread_cond_signal(&cond[num]);
	atts_array[num] = 0;
	pumps_array[num] = 0;
}

void acceptPayment(int num)
{
	// Use sleep 0.1 - and through one ATM only
	atts_array[num] = 2;
	waiting_array[0] = 1;
	printf("Attender %d - is accepting payment from a car\n", num);
	sleep(1);
    pthread_cond_signal(&cond_p[num]);
	atts_array[num] = 0;
	pays_array[num] = 0;
	waiting_array[0] = 0;
	printf("Payment Done - Attender %d  \n" , num);
}

int enterStation(int num, int stime)
{
	sleep(stime);
	int *cars = malloc(sizeof(int));
	sem_getvalue(&waiting, cars);
	int free_slots = *cars;

	printf("No. of waiting slots = %d\n", free_slots);
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
	printf("No. of free pumps = %d\n", free_pumps);
	if(free_pumps == 0) // Then Wait until pump free
	{
    	sem_wait(&waiting);
		printf("Car %d - is waiting in line at no - \n", num);
		sem_wait(&pump);
		for (i = 1; i <= 3; ++i)
		{
			if(pumps_array[i] == 0)
			{
				pumps_array[i] = 1;
				break;
			}
		}
    	sem_post(&waiting);
    	return i;
	}
	else // pumps are already free to go
	{
		sem_wait(&pump);
		for (i = 1; i <= 3; ++i)
		{
			if(pumps_array[i] == 0)
			{
				pumps_array[i] = 1;
				break;
			}
		}	
		return i;	
	}
}

void goToPump(int num, int pumpnum)
{
    pthread_mutex_lock(&mutex[pumpnum]);

	printf("Car %d - is going to Pump no - %d \n", num, pumpnum);

    pthread_cond_wait(&cond[pumpnum], &mutex[pumpnum]);

    sem_post(&pump);
    pumps_array[pumpnum] = 0;

    pthread_mutex_unlock(&mutex[pumpnum]);
}

void pay(int num)
{
	int i;
	i = 1;
	printf("Car %d - is waiting for paying\n", num);

	while(atts_array[i] != 2 && pays_array[i] == 0)
	{
		for (i = 1; i <= 3; ++i)
		{
			if(atts_array[i] == 0 && pays_array[i] == 0 )
			{
				atts_array[i] = 2;
				pays_array[i] = 1;
				if(waiting_array[0] == 0)
					break;
			}
		}
	}

    pthread_mutex_lock(&mutex_p[i]);

	printf("Car %d - is paying to attender %d- \n", num, i);
    pthread_cond_wait(&cond_p[i], &mutex_p[i]);
	// pays_array[i] = 0;
    pthread_mutex_unlock(&mutex_p[i]);

}

void exitStation(int num)
{
	printf("Car %d - is exiting the station\n", num);
}