#include <stdio.h>
#include <limits.h>

void merge(int a[], int l, int m, int r)
{
	int n1 = (m-l) + 1;
	int n2 = (r-m);
	int i,j,k;
	int L[n1],R[n2];

	for (i = 0; i < n1; ++i)
		L[i] = a[l+i];

	for (j = 0; j < n2; ++j)
		R[j] = a[m+j+1];	

	i = 0,j = 0,k = l; // k = initial index of merged subarray

	while(i < n1 && j < n2)
	{
		if(L[i] <= R[j])
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

	while(i < n1)
	{
		a[k] = L[i];
		i++;
		k++;
	}

	while(j < n2)
	{
		a[k] = R[j];
		j++;
		k++;
	}

}

void mergesort(int a[], int l, int r)
{
	if(l < r)
	{
		int mid = (l+r)/2;
		mergesort(a,l,mid);
		mergesort(a,mid+1,r);
		merge(a,l,mid,r);
	}
	return;
}

int main(int argc, char const *argv[])
{
	int num;
	scanf("%d",&num);
	int i;
	int arr[100000] = {0};
	for (i = 0; i < num; ++i)
		scanf("%d",&arr[i]);

	mergesort(arr,0,num - 1);

	for (i = 0; i < num; ++i)
		printf("%d ",arr[i]);

	printf("\n");
	return 0;
}