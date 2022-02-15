# DataLab 实验分析与总结

## **Part 1** int

### 1.`bitXor()`
```C
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
    // return ~(~(x & ~y) & ~(y & ~x));
    return (~x&~y) & ~(x&y);
}
```

**分析**：
>  异或有如下几种表示方法：
>   - $p\otimes q=(p\wedge\lnot q)\vee (q\wedge\lnot p)$  
>       理解记忆：$p, q$取不同真值。
>   - $p\otimes q=(p\vee q)\wedge (\lnot p\vee \lnot q)$  
>       理解记忆：$p, q$不同为假也不同为真。  
>       等价于:   
>       - $p\otimes q=(p\vee q)\wedge \lnot(p\wedge q)$   
>       - $p\otimes q=\lnot (\lnot(p\vee q)\vee (p\wedge q))$   
>           (运用德摩根定律)
>>DeMorGan定律：(逻辑非对逻辑与、或的分配交换律)
>>$$
\lnot(p\wedge q)=(\lnot p \vee \lnot q) \\
\lnot(p\vee q)=(\lnot p \wedge \lnot q)
>>$$

可以由上述表示法推导出仅由$\wedge, \lnot$表示的异或表达式：
$$
\begin{aligned}
p\otimes q & =(p\wedge\lnot q)\vee (q\wedge\lnot p) \\
& = \lnot(p\wedge\lnot q)\wedge \lnot(q\wedge\lnot p) \\
p\otimes q & =(p\vee q)\wedge \lnot(p\wedge q) \\
& =\lnot(\lnot(p \vee \lnot q)) \wedge \lnot(p\wedge q) \\
& =\lnot(\lnot p \wedge \lnot q) \wedge \lnot(p\wedge q)
\end{aligned}
$$
因此，`x^y`可以表示为：
```C
x^y=~(~(x&~y) & ~(y&~x));   //8个运算符
x^y=~(~x&~y) & ~(x&y);      //7个运算符 
```

**总结**：  
异或用与或非表示本质上就是上面的两种表示方法，其他的表示方法都可以运用德摩根定律推出来。  
对于异或运算，应该充分理解它的性质：相同为假，相异为真。
    

### 2.`tmin()`
```C
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
   return 1<<31;
}
```

**总结**：  
简单概念，pass


### 3.`isTmax()`
```C
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {
  int y=~0;
  return !(x^~(x+1)) & !!(x^y);
}
```

**分析**：  
题目要求判断`x`是否等于`Tmax=0x7fffffff`  
对于`Tmax, Tmin`，我们可以利用其运算容易溢出的特点。对`Tmax`来说，`Tmax+1`会让它向高位进位，溢出到`Tmin`，然后对`Tmin`取反码，又会回归到`Tmax`。  
而有着同样特点的是`0xffffffff`，这个数由于位数限制，在32位整数域中表示-1。  
对于其他值`x`，加一取反码不会使其溢出，`x`与`~(x+1)`是一个关于-1中心对称的数对。例如：
| `x` | `~(x+1)` |
|-----|----------|
| 0 | -2 |
| 1 | -3 |
| 2 | -4 |
|...|

综上，我们需要:
- 判断`x`与`~(x+1)`是否相等
- 排除`0xffffffff`

最后，运用异或运算，得到正确代码。

**总结**：  
- 涉及`Tmin, Tmax`，多利用它们的溢出特性。
- 计算技巧：  
    判断两个数是否相等：`!(x^y)`


### 4.`allOddBits()`
```C
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
  int  mask=0x55;
  mask=mask+(mask<<8);
  mask=mask+(mask<<16);
  x=mask|x;
  return !~x;
}
```
**分析**：  
题目要求判断32位整数`x`的奇数位上是否全为1。  
我们可以把偶数位全置为1，然后判断x是否所有位都为1。  
这里有一个位运算的运算技巧，通过`!~x`可以判断`x`是否所有位都为1。

**总结**：  
- 判断`x`是否所有位为1：`!~x`  
- 判断`x`是否所有位为0：`!!~x`


### 5.`negate()`
```C
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x+1;
}
```

**总结**：  
基本概念，pass


### 6.`isAsciiDigit()`
```C
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int mask=0x1<<31; 
  int upper=~mask & ~0x39;
  int lower=~0x30+1;
  return !((x+upper) & mask) & !((x+lower) & mask);
}
```

**分析**：  
题目要求判断`x`是否是一个Ascii数字，这个问题抽象成判断`x`是否在一个取值范围$[x_1, x_2]$内。  
基本想法是:   
定一个上界$upper=Tmax-x_2$, 当$x>x_2$时，$x+upper$上溢；定一个下界$lower=Tmin-x_1$，当$x<x_1$时，$x+lower$下溢，利用符号位的改变来确定。  
但要注意，这样两边利用溢出来判断范围的方法只有当$x_1\le0, x_2\ge0$时才能使用，因为符号数的表示范围是$[-2^{w-1}, 2^{w-1}-1]$，upper和lower也受这个范围限制。  
比如：8位符号数，$Tmax=127, Tmin=-128, x_1=48, x_2=57$,我们可以设上界$upper=128-57=71$，当$x>57$时，发生溢出；但对于下界$lower=-127-48=-175=81$, 它本身就溢出了。  
那对于$x_1>0, x_2<0$的情况，又该怎么办呢？其实很简单，只需要取其补码，以0为溢出边界，同理判断即可。


**总结**：  
- 这个问题最开始想了很久，想了很多通过位判断的方法，但都没啥用。最后不得不上网参考了一下别人的解答，然后那叫一个顿悟啊! 有被爽到。  
方向有时候比努力更重要。当你努力了很久也没什么成果时，请自问一句：是不是方向出了问题？
- 计算技巧：  
    运用上溢、下溢以及正负值相加来改变符号位，从而判断`x`是否在某个范围内。


### 7.`conditional`
```C
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  int mask= 0x1 <<31;
  int x0=~x+1;
  // x=(((x&mask) & (x0&mask)) | ((x&mask)^(x0&mask))) >>31;
  x=((x|x0)&mask)>>31;
  return (x&y) | (~x&z);
}
```

**分析**：
1. 通过判断`x|~x+1`符号位判断`x`是否非零
2. 将符号位右移31位，通过位运算实现条件选择

**总结**：
- 注释掉的代码是我第一次写的时候写出来的，新的代码是在做`logicalNeg()`时灵感迸发得到的。
- 计算技巧：
  - 判断非零：`x|~x+1`
  - 条件选择：`(x&y) | (~x&z)`, `x`位全为1或0


8.`isLessOrEqual()`
```C
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int sub=y+~x+1;
  int mask=0x1 <<31;
  int signx=x&mask;
  int signy=y&mask;
  int signsub=sub&mask;

  return (((signx&~signy) | (~(signx^signy) & ~signsub))>>31)&0x1;
}
```

**总结**：
这个题目很简单，通过补码实现减法，然后分类确定即可。


9.`logicalNeg`
```C
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) {
  int y=~x+1;
  int mask=0x1<<31;
  x=(x&mask);
  y=(y&mask);
  return (~(x|y)>>31)&0x1;
  //return (x|(~x+1))>>31+1;
}
```

**总结**：
同7。

10.`howManyBits()`
```C
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90
 *  Rating: 4
 */
int howManyBits(int x) {
  int mask=0x1<<31;
  int cnt16, cnt8, cnt4, cnt2, cnt1, cnt0;
  mask=(mask&x)>>31;
  x=(mask&~x) | (~mask&x);
  cnt16=!!(x>>16)<<4;
  x=x>>cnt16;
  cnt8=!!(x>>8)<<3;
  x=x>>cnt8;
  cnt4=!!(x>>4)<<2;
  x=x>>cnt4;
  cnt2=!!(x>>2)<<1;
  x=x>>cnt2;
  cnt1=!!(x>>1);
  x=x>>cnt1;
  cnt0=x;
  return cnt16+cnt8+cnt4+cnt2+cnt1+cnt0+1;
}
```

**总结**：  
这个题目我参考了网上的答案。:sob:无奈，太菜了。  
>如果是一个正数，则需要找到它最高的一位（假设是n）是1的，再加上符号位，结果为n+1；  
>如果是一个负数，则需要知道其最高的一位是0的。 
>（例如4位的1101和三位的101补码表示的是一个值：-3，最少需要3位来表示）。

这个解法有两个有趣的地方：
- 通过`conditional()`中的条件选择，将负数表示转化为正数表示的问题
- 二分思想寻找最高有效位。


## **Part 2** float
1.`floatScale2()`
```C
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  unsigned mask=0xff<<23;
  unsigned exp=(uf&mask)>>23;
  unsigned sign=uf&(0x1<<31);
  unsigned rf;

  if(exp==0xff)
    rf=uf;
  else if(exp==0)
  {
    rf=(uf<<1) | sign;
  }else{
    exp=exp+1;
    rf=((exp<<23)&mask)+(uf&~mask); 
  }

  return rf;
}
```

**分析**：  
`2*f`的作用是使`f`指数加1，分为三种情况：  
1. `f`是规范化浮点数：提取指数域+1即可；
2. `f`是非规范化浮点数：让浮点数整体左移1位，再使符号位不变即可；
3. `f`是特殊值，按题目要求返回`f`即可。

**总结**：  
非规范化浮点数的指数域定义，使这个题目变得极为简单。这应该就是作者所说IEEE表示法的优点之一吧。


2.`floatFloat2Int()`
```C
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {
  int idata;
  unsigned sign=uf&(0x1<<31);
  unsigned mask=0xff<<23;
  unsigned base=0x1<<23;
  unsigned lowerbound=0x7f;
  
  unsigned exp=(uf&mask)>>23;
  if(exp<lowerbound){
    idata=0;
  }else{
    exp=exp-lowerbound;
    mask=~(((0xff<<1)+1)<<23);
    if(exp<=23){
      idata=(((uf&mask)+base)>>(23-exp));
      if(sign){
        idata=~idata+1;
      }
    }else if(exp<=30){
      idata=(((uf&mask)+base)<<(30-exp));
      if(sign){
        idata=~idata+1;
      }
    }else{
      idata=0x80000000;
    }
  }

  return idata;
}
```

**分析**：  
`float`转化为`int`分为四种情况：($exp$定义为二进制有效指数)  
1. $exp<0$，`idata=0`
2. $0\le exp\le 23$，将指数域和符号位置为0，右移小数域$23-exp$位，最后视符号取补码。
3. $23<exp\le 30$，将指数域和符号位置为0，左移小数域$30-exp$位，最后视符号取补码。($exp\le 30$而不是$exp\le 31$是因为有一个前导1，即`base`)
4. $exp>30$，无法表示，取`0x80000000`。

**总结**：
只要理解了浮点数的表示，不难。


3.`floatPower2`
```C
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
    int lower=-126-23;
    int upper=127;
    unsigned power;
    int exp;

    if(x<lower)
    {
      power=0; 
    }else if(x>upper)
    {
      power=0xff<<23;
    }else{
      exp=x+0x7f;
      if(x<0){
        power=0x1<<22>>(-x);
      }else{
        power=exp<<23;
      }
    }
    return power;
}
```

**分析**：  
IEEE浮点数指数表示范围是(假设$k$位指数域，$n$位小数域)：  
$$[-2^{k-1}+2-n, 2^{k-1}]$$
其中下界里的那个$-n$是牺牲小数域中的精度换来的。  
知道了这个，就很简单了：
1. 当$x<lower\quad or \quad x>upper$时，按题目要求赋值
2. 当$x<-2^{k-1}+1$时，小数域中有一个1，对0x1进行适当移位得到结果。
3. 当$-2^{k-1}+2\le x\le 2^{k-1}$时，小数域为0, 对x进行偏置运算后移位即可。

**总结**：  
- IEEE浮点数指数表示范围是：  
$$[-2^{k-1}+2-n, 2^{k-1}]$$
- 如果要把表示$2^x$推广为表示$n^x$, $n\in[1, 1-\epsilon]$的话，其实是一样的道理，区别仅在于需要牺牲$x$的广度去保证能够表示$n$的精度。