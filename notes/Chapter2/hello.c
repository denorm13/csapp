#include<stdio.h>

float Sum(float a[], unsigned len)
{
	int i;
	float sum=0.0;
	for(i=0; i<=len-1; i++)
	{
		sum+=a[i];
	}
	return sum;
}

int main()
{
	float a=0.0;
	printf("%e\n", a);

	return 0;
}
