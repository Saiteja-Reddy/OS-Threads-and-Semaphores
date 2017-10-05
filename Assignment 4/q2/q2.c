#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

void *evm(void*);
void *voter(void*);

struct Booth
{
	int voters;
	int evms;
};

struct passVoter {                                                   
    int num;
    struct Booth* B;                                                              
};

struct passEvm {                                                   
    int num;        
    struct Booth* B;                                                      
};


int main()
{
	time_t t;
	int i;
	// srand((unsigned) time(&t));

	// int bos;
	// scanf("Enter no of Booths - %d", bos);
	int voters;
	struct Booth* B = malloc(sizeof(struct Booth));
	printf("Enter no of voters -");
	scanf("%d", &voters);
	int evms;
	printf("Enter no of evms - ");
	scanf("%d", &evms);
	B->voters = voters;
	B->evms = evms;

	pthread_t tid[voters];
	pthread_t e_tid[evms];

	struct passVoter *v_params[voters];
	struct passEvm *e_params[evms];

	for (i = 0; i < voters; ++i)
	{
		v_params[i] = malloc(sizeof(struct passVoter));
		v_params[i]->num = i+1;
		v_params[i]->B = B;
		// printf("%d\n", a_params[i]->num );
    	pthread_create(&tid[i], NULL, voter, v_params[i]);
	}

	for (i = 0; i < evms; ++i)
	{
		e_params[i] = malloc(sizeof(struct passEvm));
		e_params[i]->num = i+1;
		e_params[i]->B = B;
		// printf("%d\n", a_params[i]->num );
    	pthread_create(&e_tid[i], NULL, evm, e_params[i]);
	}


	for (i = 0; i < voters; ++i)
	{
    	pthread_join(tid[i], NULL);
	}
	for (i = 0; i < evms; ++i)
	{
    	pthread_join(e_tid[i], NULL);
	}


	return 0;
}

void *evm(void* arg)
{
	struct passEvm *params = arg;
	int num = params->num;
	struct Booth* B = params->B;

	printf("This is evm no. %d\n", num );
}

void *voter(void* arg)
{
	struct passVoter *params = arg;
	int num = params->num;
	struct Booth* B = params->B;

	printf("This is voter no. %d\n", num );

}