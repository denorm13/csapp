#include<stdio.h>

void multstore(long , long ,long *);

int main()
{
    long d;
    multstore(2, 3, &d);
    printf("2*3=%ld", d);
    return 0;
}

int mult2(long a, long b)
{
    long t;
    t=a+b;
    return t;
}