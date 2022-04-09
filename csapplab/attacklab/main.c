#include<stdio.h>

int main()
{
	char buf[100];
	unsigned x=0x12345678;
	sprintf(buf, "%.8x", x);
	printf("%s\n", buf);
	return 0;
}
