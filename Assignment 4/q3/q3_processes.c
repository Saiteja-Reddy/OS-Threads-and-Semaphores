#include <stdio.h>
#include <limits.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>

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

void sharedmergesort(int a[], int l, int r)
{
	// if(l < r)
	// {
	// 	int mid = (l+r)/2;



	// 	mymergesort(a,l,mid);

	// 	mymergesort(a,mid+1,r);

	// 	merge(a,l,mid,r);

	// }

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
		return;
	}


	if (l < r)
	{

		int mid = (l + r) / 2;

		pid_t pid1;
		pid_t pid2;
		int i;
		int status;

		if ((pid1 = fork()) < 0) {
			perror("fork Failed");
			exit(1);
		}

		if (pid1 != 0)
		{
			if ((pid2 = fork()) < 0) {
				perror("fork Failed");
				exit(1);
			}
		}

		if (pid1 == 0)
		{
			// printf("This is child 1.\n");
			// for(i = l; i <= r; i++) {
			// 	printf("1 - %d\n", a[i]);
			// }

			sharedmergesort(a, l, mid);


			/* Detach the shared memory */
			if (shmdt(a) == -1) {
				perror("shmdt failed");
			}
			exit(1);
		}
		if (pid2 == 0)
		{
			// printf("This is child 2.\n");
			// for(i = l; i <= r; i++) {
			// 	printf("2 - %d\n", a[i]);
			// }

			sharedmergesort(a, mid + 1, r);


			/* Detach the shared memory */
			if (shmdt(a) == -1) {
				perror("shmdt failed");
			}
			exit(1);
		}
		else
		{
			waitpid(pid1, &status, 0);
			waitpid(pid2, &status, 0);

			merge(a, l, mid, r);

			// printf("Done CHilds and ended parent\n");
		}

	}
	return;
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

	sharedmergesort(sharedArray, 0, num - 1);

	for (i = 0; i < num; ++i)
		printf("%d ",sharedArray[i]);
	printf("\n");

	/* Detach the shared memory */
	if (shmdt(sharedArray) == -1) {
		perror("shmdt failed");
	}

	return 0;
}