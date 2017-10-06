#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct Booth
{
	int id;
	int voters;
	int evms;
	int voter_it;
	int evm_it;
	int evm_ready;
	pthread_mutex_t mutex;
	pthread_mutex_t mutex_wait;
	pthread_cond_t cond;
	int final;
}booth;

void *evm(void*);
void *voter(void*);
void voter_wait_for_evm(booth*, int);
void polling_ready_evm(booth*, int , int);

#define RED   "\x1B[31m"
#define RESET "\x1B[0m"

void *booth_init(void* arg)
{
	booth* B = (booth*)arg;
    pthread_mutex_init(&(B->mutex), NULL);
    pthread_mutex_init(&(B->mutex_wait), NULL);
    pthread_cond_init(&(B->cond), NULL);
    pthread_t tid;
    int i;
    for (i = 0; i < B->voters; ++i)
    	pthread_create(&tid, NULL, voter, B);

    sleep(1);

    for (i = 0; i < B->evms; ++i)
    	pthread_create(&tid, NULL, evm, B);

    sleep(1);
}

int main()
{
	time_t t;
	int i;
	srand((unsigned) time(&t));

	int bos;
	printf("Enter no of Booths - ");
	scanf("%d", &bos);
	struct Booth* B[bos];
	int evms;
	int voters;
	for (i = 0; i < bos; ++i)
	{
		B[i] = malloc(sizeof(struct Booth));
		// printf("Enter no of voters -");
		scanf("%d", &voters);
		// printf("Enter no of evms - ");
		scanf("%d", &evms);
		B[i]->voters = voters;
		B[i]->evms = evms;
		B[i]->id = i+1;
		B[i]->final = 0;
	}
    pthread_t tid;
	for (i = 0; i < bos; ++i)
	{
		pthread_create( &tid, NULL, booth_init , B[i]);
	}
	pthread_exit(NULL);
	return 0;
}

void *evm(void* arg)
{
	booth* B = (booth*)arg;
	int e_id = (*B).evm_it++;
	int count = 2;
	// printf("This is evm no. %d of %d Booth \n", e_id + 1, B->id);
	while((*B).voters > 0)
	{
		polling_ready_evm(B, e_id+1, count);
	}
}

void *voter(void* arg)
{
	booth* B = (booth*)arg;
	int v_id = (*B).voter_it++;
	// printf("This is voter no. %d of %d Booth \n",v_id + 1, B->id);
	voter_wait_for_evm(B, v_id+1);
}

void voter_wait_for_evm(booth* B, int v_id)
{
	pthread_cond_wait(&(B->cond), &(B->mutex_wait));
	printf("Voter %d at Booth %d got allocated EVM %d\n", v_id, B->id, B->evm_ready);
	pthread_mutex_unlock(&(B->mutex_wait));
	return;
}

void polling_ready_evm(booth* B, int e_id, int slots)
{
	pthread_mutex_lock(&(B->mutex));
	int i;
	B->evm_ready = e_id;
	if(B->voters <= 0)
	{
		pthread_mutex_unlock(&(B->mutex));
		return;
	}

	printf("EVM %d at Booth %d is free with slots = %d\n", e_id, B->id, slots);
	// printf("Voters Available - %d\n", B->voters);
	for (i = 0; i < slots; ++i)
	{
		if(B->voters <= 0)
		{
			// printf("Here\n");
			break;
		}
		(*B).voters--;
		// printf("Decremented\n");
		pthread_cond_signal(&(B->cond));
		sleep(0.1);
	}
	pthread_mutex_unlock(&(B->mutex));
	printf("EVM %d at Booth %d is moving for voting stage\n", e_id, B->id);
	sleep(0.1);
	printf("EVM %d at Booth %d finished voting phase\n", e_id, B->id);
	if(B->voters == 0)
		printf(RED "Voters at Booth %d are done with voting\n" RESET, B->id);
	return;
}

// void voter_in_slot