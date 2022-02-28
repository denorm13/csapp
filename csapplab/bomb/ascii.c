/* ascii.c -- trans a string of specific format into ascii. 
 * author:  li jingyao
 * usage:
 *          1. store the string in ./hexstr
 *          2. complier and run ascii.c
 * discription:(string format)
 *              0x42    0x6f    0x72    0x64    0x65    0x72    0x20    0x72
 *              ...
 *              hex by bytes, copied directly from gdb
 */
#include<stdio.h>
#include<stdlib.h>

char *GetString(void);
char hex2char(char hex);

int main()
{
    char *str=GetString();
    printf("%s\n", str);
    return 0;
}


char *GetString(void)
{
    FILE *file=fopen("./hexstr", "r");
    char hex[4];
    char ch;
    char *str=NULL;
    int len=1;

    ch=fgetc(file);
    while(ch!=EOF)
    {
        if(ch=='x'){
            fgets(hex, 4, file);
            if(hex[2]==' ' || hex[2]=='\n'){
                str=realloc(str, len);
                str[len-1]=16*hex2char(hex[0])+hex2char(hex[1]);
                len++;
            }
        }
        ch=fgetc(file);
    }
    str[len-1]='\0';
    return str;
}

char hex2char(char hex)
{
    if(hex <='9')
        return hex-'0';
    else
        return 10+hex-'a';

}
