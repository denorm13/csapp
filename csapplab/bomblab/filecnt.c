/* filecnt.c -- cnt how many chars the file has.*/
#include<stdio.h>
#include<stdlib.h>

int CntFile(FILE *);

int main()
{
    char *fname="./hexstr";
    FILE *file=fopen(fname, "r");
    printf("File totally has %d chars.\n", CntFile(file));
    return 0;
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