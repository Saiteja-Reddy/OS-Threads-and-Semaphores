#include <stdio.h>
#include <limits.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<pthread.h>


key_t key = IPC_PRIVATE; /* This is needed */

void merge(int a[], int l, int m, int r)
{
	int n1 = (m - l) + 1;
	int n2 = (r - m);
	int i, j, k;
	int L[n1], R[n2];

	for (i = 0; i < n1; ++i)
		L[i] = a[l + i];

	for (j = 0; j < n2; ++j)
		R[j] = a[m + j + 1];

	i = 0, j = 0, k = l; // k = initial index of merged subarray

	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			a[k] = L[i];
			i++;
		}
		else
		{
			a[k] = R[j];
			j++;
		}
		k++;
	}

	while (i < n1)
	{
		a[k] = L[i];
		i++;
		k++;
	}

	while (j < n2)
	{
		a[k] = R[j];
		j++;
		k++;
	}

}

// void mymergesort(int a[], int l, int r)
// {
// 	if(l < r)
// 	{
// 		int mid = (l+r)/2;
// 		mymergesort(a,l,mid);
// 		mymergesort(a,mid+1,r);
// 		merge(a,l,mid,r);
// 	}
// 	return;
// }

void selectionsort(int arr[], int num)
{
	int i;
	int count = num;
	int min, minpos;
	int temp;
	int j;
	for (i = 0; i < count; ++i)
	{
		min = INT_MAX;
		for (j = i; j < count; ++j)
		{
			if (min > arr[j])
			{
				min = arr[j];
				minpos = j;
			}
		}
		temp = arr[i];
		arr[i] = arr[minpos];
		arr[minpos] = temp;
	}
	return;
}

struct readThreadParams {                                                   
    int *arr; 
    int left;                                                             
    int right;                                                              
}; 

void *sharedmergesort(void *arg)
{
	 struct readThreadParams *params = arg;
	 int r = params->right;
	 int l = params->left;
	 int *a = params->arr;

	int size = r - l + 1;
	if (size < 5)
	{
		int arr[5];
		int j = 0;
		int i;
		for (i = l; i < r; ++i)
		{
			arr[j++] = a[i];
		}
		selectionsort(arr, size);
		j = 0;
		for (i = l; i < r; ++i)
		{
			arr[j++] = a[i];
		}
	}


	if (l < r && size >= 5)
	{

		struct readThreadParams *childone_para = malloc(sizeof(struct readThreadParams));
		struct readThreadParams *childtwo_para = malloc(sizeof(struct readThreadParams));


		int mid = (l + r) / 2;

		pid_t pid1;
		pid_t pid2;
		int i;
		int status;

    pthread_t tid1;
    pthread_t tid2;

		childone_para->arr = a;
		childone_para->left = l;
		childone_para->right = mid;

		childtwo_para->arr = a;
		childtwo_para->left = mid+1;
		childtwo_para->right = r;		

    pthread_create(&tid1, NULL, sharedmergesort, childone_para);
    pthread_create(&tid2, NULL, sharedmergesort, childtwo_para);

	// int *a = malloc(sizeof(int));
	// *a = 1;
	// int *b = malloc(sizeof(int));
	// *b = 2;

 //    pthread_create(&tid1, NULL, testfunc, a);
 //    pthread_create(&tid2, NULL, testfunc, b);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    free(childone_para);
    free(childtwo_para);

	merge(a, l, mid, r);

	}
}


int main(int argc, char const *argv[])
{
	int num;
	scanf("%d", &num);

	int *sharedArray;
	int shmid, i, j, k, status;
	int SHMSIZE = num;

	/* Create a shared Memory segment of required size and get its id */
	shmid = shmget(key, sizeof(int) * SHMSIZE, IPC_CREAT | 0666);
	if (shmid == -1) {
		perror("Shmget failed");
		exit(1);
	}

	/* Attach the shared memory Segment to the given pointer */
	sharedArray = shmat(shmid, 0, 0);
	if (sharedArray == (void *) - 1) {
		perror("Shmat failed");
		exit(1);
	}

	/* Modify the contents before fork */
	printf("Enter the contents of array:\n");
	for (i = 0; i < SHMSIZE; i++) {
		scanf("%d", &sharedArray[i]);
	}

	struct readThreadParams *params = malloc(sizeof(struct readThreadParams));
	params->arr = sharedArray;
	params->left = 0;
	params->right = num - 1;	

	sharedmergesort(params);

	for (i = 0; i < num; ++i)
		printf("%d ",sharedArray[i]);
	printf("\n");

	/* Detach the shared memory */
	if (shmdt(sharedArray) == -1) {
		perror("shmdt failed");
	}

	return 0;
}