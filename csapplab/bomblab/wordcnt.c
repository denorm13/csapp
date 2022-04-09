#include<stdio.h>
#include<stdlib.h>
#define LEN 40 

int CntFile(FILE *);

int main()
{
    char fname[LEN];
    gets(fname);
    FILE *file=fopen(fname, "r");
    printf("Input totally %d chars.\n", CntFile(file));
    return 0;
}

int CntStr(char *str)
{
    int cnt=0;
    while(str)
    {
        cnt++;
    }
    return cnt;
}

int CntFile(FILE *file)
{
    char ch;
    char *str=NULL;
    int len=1;
    ch=fgetc(file);
    while(ch!=EOF)
    {
        str=(char *)realloc(str, len+1);
        str[len-1]=ch;
        len++;
        ch=fgetc(file);
    }
    str[len-1]='\0';
    return len-1;
}