# bomb 实验分析

## 1. Intro

这个实验要求用逆向技术破解`./bomb`中的6个字符串。  
基本思路就是使用`gdb`反汇编对应源文件中的字符串处理函数，然后找出对应的字符串。  
由于有`bomb.c`的帮助，我们可以很简单地确定函数的地址，这降低了不少难度。  
废话少说，让我们开始吧！
> 作为辅助，我写了一个小程序，用于将gdb中十六进制表示的字节机器码转化为一串ascii码并打印出来

## 2. Bomb

### Phase 1

使用gdb反汇编函数`phase_1()`得到如下汇编代码：
```x86asm
; void phase_1(char *input);
; input in %rdi
0x400ee0 <+0>:     sub    $0x8,%rsp
0x400ee4 <+4>:     mov    $0x402400,%esi
0x400ee9 <+9>:     callq  0x401338 <strings_not_equal>
0x400eee <+14>:    test   %eax,%eax
0x400ef0 <+16>:    je     0x400ef7 <phase_1+23>
0x400ef2 <+18>:    callq  0x40143a <explode_bomb>
0x400ef7 <+23>:    add    $0x8,%rsp
0x400efb <+27>:    retq
```

稍加观察，看到调用`<strings_not_equal>`前有一次参数赋值`mov $0x402400, %esi`  
于是猜测`0x402400`是字符串的地址，利用gdb查看`0x402400`处的字节表示。
```
(gdb) x /55xb 0x402400
0x402400:       0x42    0x6f    0x72    0x64    0x65    0x72    0x20    0x72
0x402408:       0x65    0x6c    0x61    0x74    0x69    0x6f    0x6e    0x73
0x402410:       0x20    0x77    0x69    0x74    0x68    0x20    0x43    0x61
0x402418:       0x6e    0x61    0x64    0x61    0x20    0x68    0x61    0x76
0x402420:       0x65    0x20    0x6e    0x65    0x76    0x65    0x72    0x20
0x402428:       0x62    0x65    0x65    0x6e    0x20    0x62    0x65    0x74
0x402430:       0x74    0x65    0x72    0x2e    0x00    0x00    0x00
```
将这段机器码使用`ascii`转化为ascii码，得到：  
`Border relations with Canada have never been better.`

不出所料，这就是第一个字符串。但找到就满足了么？让我来对`phase_1()`进行一个逆的向！  
先来看一看`string_not_equal()`是怎么工作的：
```x86asm
; bool string_not_equal(char *input, char * str)
; input in %rdi, str in %rsi
; 根据phase_1()中调用函数之前的行为以及此函数末尾的返回判断得出
0x401338 <+0>:     push   %r12
0x40133a <+2>:     push   %rbp
0x40133b <+3>:     push   %rbx
0x40133c <+4>:     mov    %rdi,%rbx                         ; input in %rbx, str in %rbp
0x40133f <+7>:     mov    %rsi,%rbp
0x401342 <+10>:    callq  0x40131b <string_length>          ; int string_length(char *str);这是一个求字符串长度的函数
0x401347 <+15>:    mov    %eax,%r12d                        ; len(input) in %r12d
0x40134a <+18>:    mov    %rbp,%rdi
0x40134d <+21>:    callq  0x40131b <string_length>          ; len(str) in %eax
0x401352 <+26>:    mov    $0x1,%edx                         ; ret=1
0x401357 <+31>:    cmp    %eax,%r12d                        ; 比较字符串长度，若不相等，直接返回1
0x40135a <+34>:    jne    0x40139b <strings_not_equal+99>
0x40135c <+36>:    movzbl (%rbx),%eax                       ; grand-do型while循环前置判断
0x40135f <+39>:    test   %al,%al                           ; 将input第一个字符读入%al， 判断其是否为'\0'
0x401361 <+41>:    je     0x401388 <strings_not_equal+80>
0x401363 <+43>:    cmp    0x0(%rbp),%al
0x401366 <+46>:    je     0x401372 <strings_not_equal+58>
0x401368 <+48>:    jmp    0x40138f <strings_not_equal+87>   ; 比较input[i]和str[i], 不相等则跳出循环
0x40136a <+50>:    cmp    0x0(%rbp),%al                     ; <+50>-<71>是循环体的主要部分
0x40136d <+53>:    nopl   (%rax)
0x401370 <+56>:    jne    0x401396 <strings_not_equal+94>
0x401372 <+58>:    add    $0x1,%rbx
0x401376 <+62>:    add    $0x1,%rbp
0x40137a <+66>:    movzbl (%rbx),%eax
0x40137d <+69>:    test   %al,%al
0x40137f <+71>:    jne    0x40136a <strings_not_equal+50>   ; 循环终止条件：到达input结尾
0x401381 <+73>:    mov    $0x0,%edx
0x401386 <+78>:    jmp    0x40139b <strings_not_equal+99>
0x401388 <+80>:    mov    $0x0,%edx
0x40138d <+85>:    jmp    0x40139b <strings_not_equal+99>
0x40138f <+87>:    mov    $0x1,%edx
0x401394 <+92>:    jmp    0x40139b <strings_not_equal+99>
0x401396 <+94>:    mov    $0x1,%edx
0x40139b <+99>:    mov    %edx,%eax
0x40139d <+101>:   pop    %rbx
0x40139e <+102>:   pop    %rbp
0x40139f <+103>:   pop    %r12
0x4013a1 <+105>:   retq 
```
综合以上信息，逆向如下：
```C
int string_not_equal(char *input, char * str)
{
    int ret=1;
    if(string_length(input)!=string_length(str))
        return ret;
    
    while(input){
        if(*input!=*str){
            return ret;
        }
        input++;
        str++;
    }
    ret=0;
    return ret;
}
```
```x86asm
; int string_length(char *str);
; str in %rdi
0x40131b <+0>:     cmpb   $0x0,(%rdi)
0x40131e <+3>:     je     0x401332 <string_length+23>
0x401320 <+5>:     mov    %rdi,%rdx
0x401323 <+8>:     add    $0x1,%rdx
0x401327 <+12>:    mov    %edx,%eax
0x401329 <+14>:    sub    %edi,%eax
0x40132b <+16>:    cmpb   $0x0,(%rdx)
0x40132e <+19>:    jne    0x401323 <string_length+8>
0x401330 <+21>:    repz retq 
0x401332 <+23>:    mov    $0x0,%eax
0x401337 <+28>:    retq  
```
`string_length()`是一个简单的`while`循环结构：
```C
int string_length(char *str)
{
    int len=0;
    char *end=str;
    while(end){
        end++;
        len=end-str
    }
    return len;
}
```

逆向完了`strings_not_equal()`，再来康康他的炸弹是怎么引爆的：
```x86asm
; void explode_bomb(void)
; 这是一个无参数，无返回值的函数
0x40143a <+0>:     sub    $0x8,%rsp
0x40143e <+4>:     mov    $0x4025a3,%edi
0x401443 <+9>:     callq  0x400b10 <puts@plt>
0x401448 <+14>:    mov    $0x4025ac,%edi
0x40144d <+19>:    callq  0x400b10 <puts@plt>
0x401452 <+24>:    mov    $0x8,%edi
0x401457 <+29>:    callq  0x400c20 <exit@plt>
```
可以看出，`explode_bomb()`主要通过调用了`puts()`输出了两个字符串，最后通过`exit(8)`退出。让我们来康康`0x4025a3`和`0x4025ac`是什么...  
`ascii`破译结果如下：
```
0x4025a3:

BOOM!!!
0x4025ac:
The bomb has blown up.
```
。。。其实，如果你试过，这就是爆炸的结果:laughing:

综上所述，`phase_1()`逆向为:
```C
void phase_1(char *input)
{
    char *str="Border relations with Canada have never been better."
    int status=strings_not_equal(input, str);
    if(status)
        explode_bomb;
    return ;
}
```
`phase 1`: Border relations with Canada have never been better.  


### Phase 2
`phase_2()`反汇编得到：
```x86asm
; void phase_2(char *input)
; input in %rdi
0x400efc <+0>:     push   %rbp
0x400efd <+1>:     push   %rbx
0x400efe <+2>:     sub    $0x28,%rsp                    ; 在栈上分配了一个长0x28=40个字节的空间
0x400f02 <+6>:     mov    %rsp,%rsi                     ; 将这个数组的地址作为参数传递给<read_six_numbers>
0x400f05 <+9>:     callq  0x40145c <read_six_numbers>   ; 调用read_six_numbers读入六个整数，结果储存在分配的前24个字节里, 数组将其命名为data
0x400f0a <+14>:    cmpl   $0x1,(%rsp)                   ; 比较 data[0]和1
0x400f0e <+18>:    je     0x400f30 <phase_2+52>
0x400f10 <+20>:    callq  0x40143a <explode_bomb>       ; 不相等，引爆炸弹
0x400f15 <+25>:    jmp    0x400f30 <phase_2+52>
0x400f17 <+27>:    mov    -0x4(%rbx),%eax               ; data[i-1] -> %eax
0x400f1a <+30>:    add    %eax,%eax                     ; 2*data[i-1] -> %eax
0x400f1c <+32>:    cmp    %eax,(%rbx)                   ; 比较2*data[i-1]与data[i]
0x400f1e <+34>:    je     0x400f25 <phase_2+41>
0x400f20 <+36>:    callq  0x40143a <explode_bomb>       ; 不相等，引爆炸弹
0x400f25 <+41>:    add    $0x4,%rbx                     ; i=i+1
0x400f29 <+45>:    cmp    %rbp,%rbx                     
0x400f2c <+48>:    jne    0x400f17 <phase_2+27>
0x400f2e <+50>:    jmp    0x400f3c <phase_2+64>         ; 循环结束
0x400f30 <+52>:    lea    0x4(%rsp),%rbx                ; &data[1] -> %rbx, int i=1, 循环开始
0x400f35 <+57>:    lea    0x18(%rsp),%rbp               ; &data[6] -> %rbp, 循环结束条件
0x400f3a <+62>:    jmp    0x400f17 <phase_2+27>
0x400f3c <+64>:    add    $0x28,%rsp
0x400f40 <+68>:    pop    %rbx
0x400f41 <+69>:    pop    %rbp
0x400f42 <+70>:    retq  
```
其中，看到一个函数`read_six_numbers()`，为了搞清楚这个函数干了什么，将其反汇编：
```x86asm
; int read_six_numbers(int *src, int *dest)
; src in %rdi, dest in %rsi
0x40145c <+0>:     sub    $0x18,%rsp                      ; 在栈上分配了0x18=24个字节的空间
0x401460 <+4>:     mov    %rsi,%rdx                       ; dest in %rdx
0x401463 <+7>:     lea    0x4(%rsi),%rcx                  ; dest+1 in %rcx
0x401467 <+11>:    lea    0x14(%rsi),%rax                
0x40146b <+15>:    mov    %rax,0x8(%rsp)                  ; dest+5 -> (%rsp+8)
0x401470 <+20>:    lea    0x10(%rsi),%rax                
0x401474 <+24>:    mov    %rax,(%rsp)                     ; dest+4 -> (%rsp)
0x401478 <+28>:    lea    0xc(%rsi),%r9                   ; dest+3 -> (%r9)
0x40147c <+32>:    lea    0x8(%rsi),%r8                   ; dest+2 -> (%r8)
0x401480 <+36>:    mov    $0x4025c3,%esi                  ; 0x4025c3 -> %esi
0x401485 <+41>:    mov    $0x0,%eax                       ; 0 -> %eax
0x40148a <+46>:    callq  0x400bf0 <__isoc99_sscanf@plt> 
0x40148f <+51>:    cmp    $0x5,%eax                         ;调用sscanf()读入六个`int`
0x401492 <+54>:    jg     0x401499 <read_six_numbers+61>
0x401494 <+56>:    callq  0x40143a <explode_bomb>
0x401499 <+61>:    add    $0x18,%rsp
0x40149d <+65>:    retq 
```
可以看到，函数中有一个很重要的调用`sscanf()`, 几乎整个汇编子程序都是为了调用这个函数。`sscanf()`拥有8个参数，全都是指针。  

关于`sscanf()`：

> `int sscanf(const char *str, const char *format, ...)`  
> 与`scanf()`以`stdin`为源不同，`sscanf()`以一个字符串为源  
> 例如：
> ```C
> #include <stdio.h>
>#include <string.h>
>#include <stdlib.h>
>
>int main()
>{
>   int day, year;
>   char weekday[20], month[20], dtm[100];
>
>   strcpy( dtm, "Saturday March 25 1989" );
>   sscanf( dtm, "%s %s %d  %d", weekday, month, &day, &year );
>
>   printf("%s %d, %d = %s\n", month, day, year, weekday );
>    
>   return(0);
>}
> ```
> 上述函数产生输出：`March 25, 1989 = Saturday`

综合以上信息，可以看出，`read_six_numbers()`做的事很简单，就是从`0x4025c3`处的字符串中读入了6个`int`。  
通过`ascii`将`0x4025c3`处的字符串翻译为`ascii`码得到：`%d %d %d %d %d %d`， 这是`sscanf()`使用的格式字符串。  
因此，我们的输入字符串的格式应该是：  
`%d %d %d %d %d %d...`：六个数字，后跟任意的字符串  
`read_six_numbers()`逆向为：
```C
int read_six_numbers(char *src, int *dest)
{
    int cnt;
    cnt=sscanf(src, "%d %d %d %d %d %d", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]);
    if(cnt<=5)
        explode_bomb();
    return cnt;
}
```

综上所述，`phase_2()`逆向为：
```C
void phase_2(char *input)
{
    int *data[6];  //是6还是10? 为什么分配40个字节?
    read_six_numbers(input, data);
    for(int i=0; i<6; i++){
        if(i==0){
            if(data[i]!=1)
                explode_bomb(); 
        }else{
            if(data[i]!=2*data[i-1])
                explode_bomb();
        }
    }
    return;
}
```
通过`phase_2()`分析，不难发现，这六个数字是以1为初始值，2为倍数增长的等比数列。    
`phase 2`: 1 2 4 8 16 32[...]([...]可以是任意字符串)

### phase 3
将`phase_3()`反汇编：
```x86asm
; void phase_3(char *input)
; input in %rdi
0x400f43 <+0>:     sub    $0x18,%rsp                        ; 分配一个长0x18=24个字节的空间
0x400f47 <+4>:     lea    0xc(%rsp),%rcx                    ; %rsp+12 -> %rcx 
0x400f4c <+9>:     lea    0x8(%rsp),%rdx                    ; %rsp+8  -> %rdx
0x400f51 <+14>:    mov    $0x4025cf,%esi                    ; "%d %d" -> %esi
0x400f56 <+19>:    mov    $0x0,%eax
0x400f5b <+24>:    callq  0x400bf0 <__isoc99_sscanf@plt>    ; 熟悉的结构，调用sscanf()从input中读取两个整数, 储存在%rsp+8, %rsp+12处, 定义为data[2]
0x400f60 <+29>:    cmp    $0x1,%eax
0x400f63 <+32>:    jg     0x400f6a <phase_3+39>         
0x400f65 <+34>:    callq  0x40143a <explode_bomb>           ; 读取失败，引爆
0x400f6a <+39>:    cmpl   $0x7,0x8(%rsp)                    ; 比较data[0]与7
0x400f6f <+44>:    ja     0x400fad <phase_3+106>            ; 大于，引爆，根据比较方式，可以确定data[0]是unsigned int
0x400f71 <+46>:    mov    0x8(%rsp),%eax                    ; data[0] -> %eax
0x400f75 <+50>:    jmpq   *0x402470(,%rax,8)                ; switch(data[0]), 跳转表结构
0x400f7c <+57>:    mov    $0xcf,%eax                        ; data[0] == 0
0x400f81 <+62>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0xcf
0x400f83 <+64>:    mov    $0x2c3,%eax                       ; data[0] == 2
0x400f88 <+69>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0x2c3
0x400f8a <+71>:    mov    $0x100,%eax                       ; data[0] == 3
0x400f8f <+76>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0x100
0x400f91 <+78>:    mov    $0x185,%eax                       ; data[0] == 4
0x400f96 <+83>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0x185
0x400f98 <+85>:    mov    $0xce,%eax                        ; data[0] == 5
0x400f9d <+90>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0xce
0x400f9f <+92>:    mov    $0x2aa,%eax                       ; data[0] == 6
0x400fa4 <+97>:    jmp    0x400fbe <phase_3+123>            ; data[1] == 0x2aa
0x400fa6 <+99>:    mov    $0x147,%eax                       ; data[0] == 7
0x400fab <+104>:   jmp    0x400fbe <phase_3+123>            ; data[1] == 0x147
0x400fad <+106>:   callq  0x40143a <explode_bomb>
0x400fb2 <+111>:   mov    $0x0,%eax
0x400fb7 <+116>:   jmp    0x400fbe <phase_3+123>
0x400fb9 <+118>:   mov    $0x137,%eax                       ; data[0] == 1
0x400fbe <+123>:   cmp    0xc(%rsp),%eax                    ; data[1] == 0x137
0x400fc2 <+127>:   je     0x400fc9 <phase_3+134>
0x400fc4 <+129>:   callq  0x40143a <explode_bomb>
0x400fc9 <+134>:   add    $0x18,%rsp
0x400fcd <+138>:   retq
```
从`0x400f75 <+50>:    jmpq   *0x402470(,%rax,8)`可以看出，这里使用了跳转表的结构，即原C语言函数是一个`switch`的结构。我们从这里入手分析这段程序。  
下面是跳转表储存的地址列表：
```x86asm
(gdb) x /60xb 0x402470
0x402470:       0x7c    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x402478:       0xb9    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x402480:       0x83    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x402488:       0x8a    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x402490:       0x91    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x402498:       0x98    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x4024a0:       0x9f    0x0f    0x40    0x00    0x00    0x00    0x00    0x00
0x4024a8:       0xa6    0x0f    0x40    0x0
```
结合反汇编代码，得到对应关系：
| data[0] | case address | data[1] |
|---------|--------------|---------|
| 0 | 0x400f7c | 0xcf |
| 1 | 0x400fb9 | 0x137 |
| 2 | 0x400f83 | 0x2c3 |
| 3 | 0x400f8a | 0x100 |
| 4 | 0x400f91 | 0x185 |
| 5 | 0x400f98 | 0xce |
| 6 | 0x400f9f | 0x2aa |
| 7 | 0x400fa6 | 0x147 |

综上所述，`phase_3()`逆向为：
```C
void phase_3(char *input)
{
    unsigned int data[2];
    if(sscanf(input, "%d %d", &data[0], &data[1])<2)
        explode_bomb();
    switch(data[0]):
        case 0:
            if(data[1]!=0xcf)
                explode_bomb;
            break;
        case 1:
            if(data[1]!=0x137)
                explode_bomb;
            break;
        case 2:
            if(data[1]!=0x2c3)
                explode_bomb;
            break;
        case 3:
            if(data[1]!=0x100)
                explode_bomb;
            break;
        case 4:
            if(data[1]!=0x185)
                explode_bomb;
            break;
        case 5:
            if(data[1]!=0xce)
                explode_bomb;
            break;
        case 6:
            if(data[1]!=0x2aa)
                explode_bomb;
            break;
        case 7:
            if(data[1]!=0x147)
                explode_bomb;
            break;
        default:
            explode_bomb;
}
```

综合上述信息分析：`phase_3()`将前缀为2个整数的字符串读入，检验2个前缀整数是否符合`switch`所定义的关系。经过分析，总共有8种可能字符串模式：  
`phase 3`:
```
0 207 [...]
1 311 [...]
2 707 [...]
3 256 [...]
4 389 [...]
5 206 [...]
6 682 [...]
7 327 [...]
```

### phase 4
```x86asm
; void phase_4(char *input)
; input in %rdi
0x40100c <+0>:     sub    $0x18,%rsp                      ; 分配一个长0x18=24字节的空间
0x401010 <+4>:     lea    0xc(%rsp),%rcx                  ; ...
0x401015 <+9>:     lea    0x8(%rsp),%rdx
0x40101a <+14>:    mov    $0x4025cf,%esi
0x40101f <+19>:    mov    $0x0,%eax
0x401024 <+24>:    callq  0x400bf0 <__isoc99_sscanf@plt>  ; 熟悉的代码！读入两个无符号整数data[2]
0x401029 <+29>:    cmp    $0x2,%eax
0x40102c <+32>:    jne    0x401035 <phase_4+41>           ; 读入错误，炸
0x40102e <+34>:    cmpl   $0xe,0x8(%rsp)                  ; data[0] <= 0xe
0x401033 <+39>:    jbe    0x40103a <phase_4+46>           
0x401035 <+41>:    callq  0x40143a <explode_bomb>         ; 大于，炸
0x40103a <+46>:    mov    $0xe,%edx                       ; 14 -> %edx
0x40103f <+51>:    mov    $0x0,%esi                       ; 0  -> %esi
0x401044 <+56>:    mov    0x8(%rsp),%edi                  ; data[0] -> %edi
0x401048 <+60>:    callq  0x400fce <func4>                ; 调用fun4(), 有3个参数，data[0], 0, 14
0x40104d <+65>:    test   %eax,%eax                       ; 检验返回值是否为0
0x40104f <+67>:    jne    0x401058 <phase_4+76>           ; 不为0，炸
0x401051 <+69>:    cmpl   $0x0,0xc(%rsp)                  ; data[1] == 0
0x401056 <+74>:    je     0x40105d <phase_4+81>
0x401058 <+76>:    callq  0x40143a <explode_bomb>         ; 不相等，炸
0x40105d <+81>:    add    $0x18,%rsp
0x401061 <+85>:    retq 
```

`phase_4()`的代码比较简单，可以直接根据流程写出逆向代码：
```C
void phase_4(char *input)
{
    int data[2];
    int value;
    if(sscanf(input, "%d %d", &data[0], &data[1])!=2){
        explode_bomb();
    }
    if(data[0]<=14){
        value=fun4((int)data[0], 0, 14);
    }else{
        explode_bomb();
    }
    if(value!=0 || data[1]!=0){
        explode_bomb();
    }
}
```
根据`phase_4()`的代码，已经可以确定`phase 4`的一部分，它和`phase 3`模式相似，第二个整数是0，而第一个整数要使得`fun4()`函数返回0。下面对`fun4()`进行逆向：
```x86asm
; int func4(int data, int x, int y)
; data in %edi, x in %esi, y in %edx
0x400fce <+0>:     sub    $0x8,%rsp             ; 分配8个字节的空间
0x400fd2 <+4>:     mov    %edx,%eax             ; y     -> %eax
0x400fd4 <+6>:     sub    %esi,%eax             ; y-x   -> %eax
0x400fd6 <+8>:     mov    %eax,%ecx             ; y-x   -> %ecx
0x400fd8 <+10>:    shr    $0x1f,%ecx            ; (y-x)>>31 -> %ecx  符号位
0x400fdb <+13>:    add    %ecx,%eax             ; 若y<x, y-x+1, 反之y-x -> %eax
0x400fdd <+15>:    sar    %eax                  
0x400fdf <+17>:    lea    (%rax,%rsi,1),%ecx    ; (%eax/2)+x -> %ecx, 定义为value
0x400fe2 <+20>:    cmp    %edi,%ecx             ; 将value和data比较
0x400fe4 <+22>:    jle    0x400ff2 <func4+36>   ; 若<=, 跳转
0x400fe6 <+24>:    lea    -0x1(%rcx),%edx       ; 若>，y=value-1, 递归
0x400fe9 <+27>:    callq  0x400fce <func4>
0x400fee <+32>:    add    %eax,%eax             ; 大于返回，ret=2*ret
0x400ff0 <+34>:    jmp    0x401007 <func4+57>
0x400ff2 <+36>:    mov    $0x0,%eax
0x400ff7 <+41>:    cmp    %edi,%ecx             ; 将value和data再比较
0x400ff9 <+43>:    jge    0x401007 <func4+57>   ; 等于返回，ret=0
0x400ffb <+45>:    lea    0x1(%rcx),%esi        ; 小于，x=value+1，递归
0x400ffe <+48>:    callq  0x400fce <func4>
0x401003 <+53>:    lea    0x1(%rax,%rax,1),%eax ; 小于返回，ret=2*ret+1
0x401007 <+57>:    add    $0x8,%rsp
0x40100b <+61>:    retq   
```
```C
// error unknown
int func4(unsigned int data, int x, int y)
{
    int value;
    if(x<y){
        value=y-x;
    }else{
        value=y-x+1;
    }
    value=value/2+x;
    if(value==date){
        return 0
    }else if(value>data){
        return 2*func4(data, x, value-1);
    }else{
        return 2*func4(data, value+1, y)+1;
    }
}
```
由上述逆向代码，这个字符串同样也是一个前缀整数字符串，只有当字符串前缀整数满足以下两个条件时，炸弹不会爆炸：  
1. `fun4(data[0], 0, 14)==0`
2. `data[1]=0`

对于条件1，由我的逆向代码，可以知道，若要返回0，必须只能在`value>data`情况下进行递归调用，否则返回值至少是1。  
因此，由递归过程可以得到，`data[0]=0, 1, 3, 7`

> 一个很悲伤的Bug:  
> **单操作数的移位指令**  
> `sar %eax` 相当于`sar 1, %eax`  
> 而我在逆向`func4()`的过程中一直以为`sar %eax`是以`%cl`中的值为操作值。。。  
> 所以这导致我对着汇编代码干瞪眼了好长时间。。。
> ...  
> 这找bug, 多是一件美事啊。。。

总之：  
`phase 4`: [0,1,3,7] 0 [...]

### phase 5

`phase_5`反汇编代码：
```x86asm
; void phase_5(char *input)
; input in %rdi
0x401062 <+0>:     push   %rbx
0x401063 <+1>:     sub    $0x20,%rsp                    ; 分配0x20=32个字节空间
0x401067 <+5>:     mov    %rdi,%rbx                     ; input -> %rbx
0x40106a <+8>:     mov    %fs:0x28,%rax                 ; 栈破坏检测, canary value
0x401073 <+17>:    mov    %rax,0x18(%rsp)               ; canary value位于%rsp+24处
0x401078 <+22>:    xor    %eax,%eax
0x40107a <+24>:    callq  0x40131b <string_length>      ; 计算input长度
0x40107f <+29>:    cmp    $0x6,%eax
0x401082 <+32>:    je     0x4010d2 <phase_5+112>        ; 不相等，爆炸
0x401084 <+34>:    callq  0x40143a <explode_bomb>
0x401089 <+39>:    jmp    0x4010d2 <phase_5+112>
0x40108b <+41>:    movzbl (%rbx,%rax,1),%ecx            ; (unsigned int)input[i] -> %ecx
0x40108f <+45>:    mov    %cl,(%rsp)                    
0x401092 <+48>:    mov    (%rsp),%rdx                   
0x401096 <+52>:    and    $0xf,%edx                     ; input[i]%16   -> %edx
0x401099 <+55>:    movzbl 0x4024b0(%rdx),%edx           ; int *data=0x4024b0    data[input[i]%16]   -> %edx(define as temp)
0x4010a0 <+62>:    mov    %dl,0x10(%rsp,%rax,1)         ; (char)temp    -> %rsp+16
0x4010a4 <+66>:    add    $0x1,%rax                     ; i++
0x4010a8 <+70>:    cmp    $0x6,%rax                     ; 循环结束条件，i==6
0x4010ac <+74>:    jne    0x40108b <phase_5+41>         ; 这是一个while循环
0x4010ae <+76>:    movb   $0x0,0x16(%rsp)               ; 给%rsp+10处的字符串添加'\0'，说明通过一个特别的映射，input前6个元素映射为一个字符串str
0x4010b3 <+81>:    mov    $0x40245e,%esi                
0x4010b8 <+86>:    lea    0x10(%rsp),%rdi
0x4010bd <+91>:    callq  0x401338 <strings_not_equal>  ; 将映射字符串与0x40245e处字符串进行比较
0x4010c2 <+96>:    test   %eax,%eax
0x4010c4 <+98>:    je     0x4010d9 <phase_5+119>
0x4010c6 <+100>:   callq  0x40143a <explode_bomb>
0x4010cb <+105>:   nopl   0x0(%rax,%rax,1)
0x4010d0 <+110>:   jmp    0x4010d9 <phase_5+119>
0x4010d2 <+112>:   mov    $0x0,%eax
0x4010d7 <+117>:   jmp    0x40108b <phase_5+41>
0x4010d9 <+119>:   mov    0x18(%rsp),%rax
0x4010de <+124>:   xor    %fs:0x28,%rax
0x4010e7 <+133>:   je     0x4010ee <phase_5+140>
0x4010e9 <+135>:   callq  0x400b30 <__stack_chk_fail@plt>   ;函数返回前的栈破坏检测
0x4010ee <+140>:   add    $0x20,%rsp
0x4010f2 <+144>:   pop    %rbx
0x4010f3 <+145>:   retq
```
经过分析可以看出，`phase_5()`做以下几件事情：
1. 读入输入字符串的前6个字符
2. 将字符数组映射为一个长为6字符串
3. 将此字符串与设置好的字符数组(称其为标准字符串)比较，若相等，则通过

- 预先设置好的字符数组存放在`0x40245e`处：  
    ```
    0x40245e:       0x66    0x6c    0x79    0x65    0x72    0x73    0x00
    // 翻译为flyers
    ```

- 映射关系为：
    ```
    str[i]=*(0x4024b0+input[i]%0x10)
    // 其中，x为输入字符串的字符，y是映射后的字符串
    ```

通过自己写的小程序`check`找出`flyers`中每个字符相对于`0x4024b0`的偏移地址:
```bash
$ ./ascii | ./check 
e:0x5
r:0x6
s:0x7
f:0x9
y:0xe
l:0xf
```
现在我们只需要将这几个对应的偏移地址按照标准字符串的顺序，加上`0x10`的整数倍，就得到我们的输入字符串：
`0xk9 0xkf 0xke 0xk5 0xk6 0xk7`  
当然，每一个k值不一定要相等。

`phase 5`:
```
k=2: )/.%&'[...]
k=3: 9?>567[...]
...
```

### phase 6

Finally! 终于到了Boss关，来逆向这个巨长的程序吧！

```x86asm
; void phase_6(char *input)
; input in %rdi
0x4010f4 <+0>:     push   %r14
0x4010f6 <+2>:     push   %r13
0x4010f8 <+4>:     push   %r12
0x4010fa <+6>:     push   %rbp
0x4010fb <+7>:     push   %rbx
0x4010fc <+8>:     sub    $0x50,%rsp                    ; 分配长0x50=80个字节的空间
0x401100 <+12>:    mov    %rsp,%r13                     ; %rsp  -> %r13 int *p1=data;
0x401103 <+15>:    mov    %rsp,%rsi                     ; %rsp  -> %rsi
0x401106 <+18>:    callq  0x40145c <read_six_numbers>   ; 从input读入6个数字，储存在(%rsp, %rsp+0x18)处，unsigned int data[6]
0x40110b <+23>:    mov    %rsp,%r14                     ; %rsp  -> %r14 int *p2=data;
----------------------------------------------------
0x40110e <+26>:    mov    $0x0,%r12d                    ; 0     -> %r12 int i=0     循环初始化条件
0x401114 <+32>:    mov    %r13,%rbp                     ; %rsp  -> %rbp int *p3=p1
0x401117 <+35>:    mov    0x0(%r13),%eax               
0x40111b <+39>:    sub    $0x1,%eax                      
0x40111e <+42>:    cmp    $0x5,%eax                     
0x401121 <+45>:    jbe    0x401128 <phase_6+52>         ; *p1-1<=5, 否则爆炸
0x401123 <+47>:    callq  0x40143a <explode_bomb>
0x401128 <+52>:    add    $0x1,%r12d                    ; i++
0x40112c <+56>:    cmp    $0x6,%r12d                    ; i==6  外层循环结束条件
0x401130 <+60>:    je     0x401153 <phase_6+95>
0x401132 <+62>:    mov    %r12d,%ebx                    ; i     -> %ebx int j=i
0x401135 <+65>:    movslq %ebx,%rax                    
0x401138 <+68>:    mov    (%rsp,%rax,4),%eax            ; data[j]   -> %eax
0x40113b <+71>:    cmp    %eax,0x0(%rbp)                
0x40113e <+74>:    jne    0x401145 <phase_6+81>         ; data[j]!=data[i], 否则爆炸
0x401140 <+76>:    callq  0x40143a <explode_bomb>
0x401145 <+81>:    add    $0x1,%ebx                     ; j++
0x401148 <+84>:    cmp    $0x5,%ebx                     
0x40114b <+87>:    jle    0x401135 <phase_6+65>         ; j<=5  循环判断条件
0x40114d <+89>:    add    $0x4,%r13                     ; i>5, p1+1
0x401151 <+93>:    jmp    0x401114 <phase_6+32>
--------------------------------------------------
0x401153 <+95>:    lea    0x18(%rsp),%rsi               ; p2+6      -> %rsi     int *end=p2+6
0x401158 <+100>:   mov    %r14,%rax                     ; p2        -> %rax
0x40115b <+103>:   mov    $0x7,%ecx
0x401160 <+108>:   mov    %ecx,%edx
0x401162 <+110>:   sub    (%rax),%edx                   
0x401164 <+112>:   mov    %edx,(%rax)                   ; *p2=7-*p2
0x401166 <+114>:   add    $0x4,%rax                     ; p2+1
0x40116a <+118>:   cmp    %rsi,%rax                     ; p2==end   循环结束条件, 这个一个do-while循环
0x40116d <+121>:   jne    0x401160 <phase_6+108>        ; 这个循环将data中的值进行变换data[i]=7-data[i]
--------------------------------------------------
0x40116f <+123>:   mov    $0x0,%esi                     ; int i=0
0x401174 <+128>:   jmp    0x401197 <phase_6+163>
0x401176 <+130>:   mov    0x8(%rdx),%rdx                ; (0x6032d0+8)  -> %rdx    若data[i]>1, node=node->next
0x40117a <+134>:   add    $0x1,%eax                     ; %eax++
0x40117d <+137>:   cmp    %ecx,%eax
0x40117f <+139>:   jne    0x401176 <phase_6+130>        ; %eax==data[i], 循环结束条件
0x401181 <+141>:   jmp    0x401188 <phase_6+148>
0x401183 <+143>:   mov    $0x6032d0,%edx                ; 若data[i]<=1  ldata[i]=node
0x401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2)        ; 0x6032d0  -> %rsp+0x20+2*%rsi   %rsp+0x20 -> int *ldata[6]
0x40118d <+153>:   add    $0x4,%rsi                     ; i++
0x401191 <+157>:   cmp    $0x18,%rsi                
0x401195 <+161>:   je     0x4011ab <phase_6+183>        ; i==6 循环结束条件
0x401197 <+163>:   mov    (%rsp,%rsi,1),%ecx            ; data[i]   -> %ecx
0x40119a <+166>:   cmp    $0x1,%ecx                     
0x40119d <+169>:   jle    0x401183 <phase_6+143>        ; data[i] <=1
0x40119f <+171>:   mov    $0x1,%eax                     ; 1         -> %eax
0x4011a4 <+176>:   mov    $0x6032d0,%edx                ; 0x6032d0  -> %edx     node
0x4011a9 <+181>:   jmp    0x401176 <phase_6+130>
-------------------------------------------------
0x4011ab <+183>:   mov    0x20(%rsp),%rbx               ; ldata[0]  -> %rbx 
0x4011b0 <+188>:   lea    0x28(%rsp),%rax               ; &ldata[1] -> %rax     int i=1
0x4011b5 <+193>:   lea    0x50(%rsp),%rsi               ; &ldata[6] -> %rsi
0x4011ba <+198>:   mov    %rbx,%rcx                     ; %rbx      -> %rcx
0x4011bd <+201>:   mov    (%rax),%rdx                   ; ldata[i]  -> %rdx
0x4011c0 <+204>:   mov    %rdx,0x8(%rcx)                ; ldata[i]  -> *(ldata[i-1]+1)
0x4011c4 <+208>:   add    $0x8,%rax                     ; i++
0x4011c8 <+212>:   cmp    %rsi,%rax
0x4011cb <+215>:   je     0x4011d2 <phase_6+222>        ; i==6, 循环结束条件
0x4011cd <+217>:   mov    %rdx,%rcx                     
0x4011d0 <+220>:   jmp    0x4011bd <phase_6+201>
0x4011d2 <+222>:   movq   $0x0,0x8(%rdx)                ; *(ldata[5]+1)=0， NULL
-------------------------------------------------
0x4011da <+230>:   mov    $0x5,%ebp                     ; 5     -> %ebp     int i=5
0x4011df <+235>:   mov    0x8(%rbx),%rax                ; *(ldata[0]+1)         -> %rax           
0x4011e3 <+239>:   mov    (%rax),%eax                   ; (int)**(ldata[0]+1)   -> %eax        
0x4011e5 <+241>:   cmp    %eax,(%rbx)                   ; 比较*ldata[0]和(int) **(ldata[0]+1)
0x4011e7 <+243>:   jge    0x4011ee <phase_6+250>        ; *ldata[0]>=**(ldata[0]+1)             node->data >= node->next->data
0x4011e9 <+245>:   callq  0x40143a <explode_bomb>       ; 否则，引爆
0x4011ee <+250>:   mov    0x8(%rbx),%rbx                ; *(ldata[0]+1) -> %rbx                 node=node->next
0x4011f2 <+254>:   sub    $0x1,%ebp                     ; i--
0x4011f5 <+257>:   jne    0x4011df <phase_6+235>        ; i==0 循环结束条件
0x4011f7 <+259>:   add    $0x50,%rsp
0x4011fb <+263>:   pop    %rbx
0x4011fc <+264>:   pop    %rbp
0x4011fd <+265>:   pop    %r12
0x4011ff <+267>:   pop    %r13
0x401201 <+269>:   pop    %r14
0x401203 <+271>:   retq 
```
1. 第一个循环（二重循环）：  
    - 外层循环检验`data[i]`是否满足`data[i]-1<=5`
    - 内存循环检验对于`data[i]`，`data[j]`是否都不等于`data[i]`  
    data元素范围：`[1, 6]`

2. 第二个循环：  
    将data[i]进行变换：changed_data[i]=7-data[i]   
    changed_data元素范围：`[1, 6]`

3. 第三个循环：  
    - 建立了一个元素长8字节的6元素数组，可能是`long , type *`, 定义为`type ldata[6]`  
    解析0x6032d0处的信息：
    ```
    0x6032d0 <node1>:       0x000000010000014c      0x00000000006032e0
    0x6032e0 <node2>:       0x00000002000000a8      0x00000000006032f0
    0x6032f0 <node3>:       0x000000030000039c      0x0000000000603300
    0x603300 <node4>:       0x00000004000002b3      0x0000000000603310
    0x603310 <node5>:       0x00000005000001dd      0x0000000000603320
    0x603320 <node6>:       0x00000006000001bb      0x0000000000000000
    0x603330:       0x0000000000000000      0x0000000000000000
    ```
    可以看出，这是一个有着6个元素的链表（。。。元素相邻），而`node1[0]`数值足够大，不可能是一个指针（我的电脑内存16G）。  
    所以，这个链表的结构如下：  
    ```C
    struct node{
        long data;
        struct node *next;
    }
    struct node *node1; 
    struct node *node2; 
    ...
    ```
    同时，考虑到给`ldata[i]`的赋值行为，`ldata[i]`可能是一个`long * or long **`，当然，它们都是8bytes

    - 结合以上信息，这个循环根据`data[i]`对`ldata[i]`进行赋值：`ldata[i]=nodei`, 其中`nodei`是一个节点

4. 第四个循环：
    根据`ldata[i]`对3中提到的链表进行更改：`*(ldata[i-1]+1)=ldata[i]`即 `ldata[i]`是`ldata[i-1]`的后继结点。

5. 第五个循环：
    在4中对链表进行修改之后，得到一个以`ldata[0]`为头指针的链表，这个循环检验这个链表是否满足：  
    结点值 `(int)data` 值不增。

根据分析结果，逆向如下：
```C
void phase_6(char *input)
{
    unsigned int data[6];
    int i=0; 

    read_six_numbers(input, data);
    for(i=0; i<6; i++)
    {
        if(data[i]>6)
            explode_bomb();
        for(int j=i; j<=5; j++){
            if(data[j] == data[i])
                explode_bomb();
        }
    }

    do{
        data[i]=7-data[i];
        i++;
    }while(i<6);

    long *ldata[6];
    for(i=0; i<6; i++)
    {
        int cnt=i;
        if(cnt==0)
            ldata[i]=node1;
        else{
            while(cnt--)
                node1=node1->next;
            ldata[i]=node1;
        } 
    }

    for(i=1; i<6; i++)
    {
        *(ldata[i-1]+1)=ldata[i];
    }
    *(ldata[i-1]+1)=NULL;
     
    long *ptr=ldata[0]; 
    i=5; 
    do{
        if((int)ptr->data < (int)ptr->next->data)
            explode_bomb;
        ptr=ptr->next;
        i--;
    }while(i);
}

```

**分析总结**：
    根据以上结果，`phase_6()`读入6个范围在`[1, 6]`的整数，然后将这六个整数进行一个映射，最后对一个6元素链表进行排序。排序顺序由映射后的整数序列确定，排序关键字是`(int)x`,其中`x`是链表结点的值。

    根据3中得到的值，链表顺序应该是：
    ```
    0x6032f0 <node3>:       0x000000030000039c      0x0000000000603300
    0x603300 <node4>:       0x00000004000002b3      0x0000000000603310
    0x603310 <node5>:       0x00000005000001dd      0x0000000000603320
    0x603320 <node6>:       0x00000006000001bb      0x0000000000000000
    0x6032d0 <node1>:       0x000000010000014c      0x00000000006032e0
    0x6032e0 <node2>:       0x00000002000000a8      0x00000000006032f0
    ```

    根据2, 3, 4中结论，`ldata, changed_data, data`为：  
    | i | ldata[i] | changed_data | data |
    |---|----------|--------------|------|
    | 0 | 0x6032f0 | 3 | 4 |
    | 1 | 0x603300 | 4 | 3 |
    | 2 | 0x603310 | 5 | 2 |
    | 3 | 0x603320 | 6 | 1 |
    | 4 | 0x6032d0 | 1 | 6 |
    | 5 | 0x6032e0 | 2 | 5 |

`phase 6`: 4 3 2 1 6 5 [...]


## 3. End

### Congratulations!!!

```bash
$ ./bomb string 
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
Phase 1 defused. How about the next one?
That's number 2.  Keep going!
Halfway there!
So you got that one.  Try this one.
Good work!  On to the next...
Congratulations! You've defused the bomb!
```

### 彩蛋
当你没有任何准备点燃了这个炸弹，其实可以用`ctrl+c`关掉它。。。:laughing:
```bash
$ ./bomb
Welcome to my fiendish little bomb. You have 6 phases with
which to blow yourself up. Have a nice day!
^CSo you think you can stop the bomb with ctrl-c, do you?
Well...OK. :-)
```
