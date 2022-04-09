#include<stdio.h>

int func4(int data, int x, int y);

int main(void)
{
	for(int i=0; i<=14; i++)
	{
		if(func4(i, 0, 14)==0)
			printf("%d ", i);
	}
	return 0;
}

int func4(int data, int x, int y)
{
    int value;
    if(x<y){
        value=y-x;
    }else{
        value=(y-x+1)/2;
    }
    value=value+x;
    if(value == data){
        return 0;
    }else if(value>data){
        return 2*func4(data, x, value-1);
    }else{
        return 2*func4(data, value+1, y)+1;
    }
}
