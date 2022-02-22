#include<stdio.h>

long mstore(long, long);

int main()
{
    long d;
    mulstore(2,3, &d);
    printf("2*3-->%ld\n", d);

    return 0;
}