#include<stdio.h>

int main()
{
    char str[128];
    int status[6]={0};
    scanf("%s", str);
    for(int i=0;i<6;i++)
    {
        for(int j=0;j<128;j++)
        {
            switch (str[j]){
                case 'f':
                    if(status[0]==0){
                        printf("f:%#x\n", j*1);
                        status[0]=1;
                    }
                    break;
                case 'l':
                    if(status[1]==0){
                        printf("l:%#x\n", j*1);
                        status[1]=1;
                    }
                    break;
                case 'y':
                    if(status[2]==0){
                        printf("y:%#x\n", j*1);
                        status[2]=1;
                    }
                    break;
                case 'e':
                    if(status[3]==0){
                        printf("e:%#x\n", j*1);
                        status[3]=1;
                    }
                    break;
                case 'r':
                    if(status[4]==0){
                        printf("r:%#x\n", j*1);
                        status[4]=1;
                    }
                    break;
                case 's':
                    if(status[5]==0){
                        printf("s:%#x\n", j*1);
                        status[5]=1;
                    }
                    break;
                default:
                    break; 
            }
        }
    }
    return 0;
}