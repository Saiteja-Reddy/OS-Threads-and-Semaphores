#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <stdlib.h>
#include <semaphore.h>

// sem_t pump;
sem_t pump;

void *car(void *arg);
void *attender(void *arg);
void enterStation(int, int);
void waitInLine(int);
void goToPump(int);
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

int main()
{
	int no;
	printf("Enter number of cars: ");
	scanf("%d", &no);
	printf("Number of cars inputed - %d\n\n", no );
    
	time_t t;
	srand((unsigned) time(&t));
	int i;
	
	pthread_t tid[no];

	pthread_t a_tid[3];

	struct passParamsAttender *a_params[3];
	// for (i = 0; i < 3; ++i)
	// {
	// 	a_params[i] = malloc(sizeof(struct passParamsAttender));
	// 	a_params[i]->num = i+1;
	// 	// printf("%d\n", a_params[i]->num );
 //    	pthread_create(&a_tid[i], NULL, attender, a_params[i]);
	// }

	// for (i = 0; i < 3; ++i)
	// {
 //    	pthread_join(a_tid[i], NULL);
	// }


	struct passParamsCar *params[no];

    sem_init(&pump,0, 3);
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

	return 0;
}

void *car(void *arg)
{
	struct passParamsCar *params = arg;
	int num = params->num;
	int stime = params->time;


	enterStation(num, stime);
	// waitInLine(num);
	// goToPump(num);
	// pay(num);
	// exitStation(num);
	printf("\n");
}

void *attender(void* arg)
{
	struct passParamsAttender *params = arg;
	int num = params->num;

	serveCar(num);
	acceptPayment(num);
	printf("\n");
}

void serveCar(int num)
{
	// Use sleep 1
	printf("Attender %d - is serving car no - \n", num);
}

void acceptPayment(int num)
{
	// Use sleep 0.1 - and through one ATM only
	printf("Attender %d - is accepting payment from car no - \n", num);
}

void enterStation(int num, int stime)
{
	sleep(stime);
	int *cars = malloc(sizeof(int));
	sem_getvalue(&pump, cars);
	int free_slots = *cars;
	printf("Already Cars = %d\n", free_slots);
	if(free_slots == 0)
		printf("Car %d - has left station - line full\n", num);
	else
	{
    	sem_wait(&pump);
		printf("Car %d - has entered station, after sleep %d\n", num, stime);
		sleep(3);
    	sem_post(&pump);
    	printf("Done - car %d\n", num);
	}
}

void waitInLine(int num)
{
	printf("Car %d - is waiting in line at no - \n", num);
}

void goToPump(int num)
{
	printf("Car %d - is going to Pump no - \n", num);
}

void pay(int num)
{
	printf("Car %d - is paying to attender no - \n", num);
}

void exitStation(int num)
{
	printf("Car %d - is exiting the station\n", num);
}