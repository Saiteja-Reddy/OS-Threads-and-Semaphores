#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#include <stdlib.h>

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
	for (i = 0; i < 3; ++i)
	{
		a_params[i] = malloc(sizeof(struct passParamsAttender));
		a_params[i]->num = i+1;
		// printf("%d\n", a_params[i]->num );
    	pthread_create(&a_tid[i], NULL, attender, a_params[i]);
	}

	for (i = 0; i < 3; ++i)
	{
    	pthread_join(a_tid[i], NULL);
	}


	// struct passParamsCar *params[no];

	// for (i = 0; i < no; ++i)
	// {
	// 	params[i] = malloc(sizeof(struct passParamsCar));
	// 	params[i]->time = rand()%3;
	// 	params[i]->num = i+1;
	// 	//printf("%d %d\n", params[i]->num, params[i]->time );
 //    	pthread_create(&tid[i], NULL, car, params[i]);
	// }

	// for (i = 0; i < no; ++i)
	// {
 //    	pthread_join(tid[i], NULL);
	// }

	return 0;
}

void *car(void *arg)
{
	struct passParamsCar *params = arg;
	int num = params->num;
	int stime = params->time;

	enterStation(num, stime);
	waitInLine(num);
	goToPump(num);
	pay(num);
	exitStation(num);
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
	printf("Attender %d - is serving car no - \n", num);
}

void acceptPayment(int num)
{
	printf("Attender %d - is accepting payment from car no - \n", num);
}

void enterStation(int num, int stime)
{
	printf("Car %d - has entered station, after sleep %d\n", num, stime);
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