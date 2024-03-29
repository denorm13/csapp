# archlab 实验分析

## 实验环境准备

### 1. 配置基本环境

archlab中需要安装的工具有如下几个：`flex`, `bison`, `tcl\tk`.   
`flex, bison`是生成汇编器yas的必要组件，大概功能是提供[语义语法分析](https://blog.csdn.net/CrazyHeroZK/article/details/87359818)的工具, `tcl\tk`可以为Y86模拟器提供GUI环境（当然， GUI不是必须的，`tcl\tk`只有编译GUI模式的模拟器时会用到，这一步花了我很长时间去调试各种bug，简直折磨，后面我会把踩的坑一一列出来）。

使用如下指令安装上述工具：
```
linux$ sudo apt-get update
linux$ sudo apt-get install flex bison
linux$ sudo apt-get install tcl tcl-dev tk tk-dev
```

至此，如果你只想使用`TTY`版本的模拟器，只需要将`sim/Makefile`中`GUIMODE, TKLIBS, TKINC`三行注释掉，然后执行
```
linux$ make clean; make
```

跳过2， 开始实验。

### 2. *编译GUI版本的模拟器

#### 2.1 修改Makefile文件

Makefile中有两个变量要修改, 按照说明找到对应文件夹修改即可：
```
TKLIBS=-L/usr/lib -ltk -ltcl
TKINC=-isystem /usr/include/tcl8.5
```
例如：  
使用`find`找到对应文件:
```
linux$ find / -iname libtcl.so
/usr/lib/x86_64-linux-gnu/libtcl.so
```
这里，把`TKLIBS=-L/usr/lib -ltk -ltcl`改为`TKLIBS=-L/usr/lib/x86_64-linux-gnu -ltk -ltcl` 

#### 2.2 修改文件

- `tcl`兼容问题

    改完Makefile就完了吗？不，当你编译时，会发现:
    ```
    psim.c:853:8: error: ‘Tcl_Interp’ {aka ‘struct Tcl_Interp’} has no member named ‘result’
    853 |  interp->result = "No arguments allowed";
        ...
    psim.c:994:56: error: ‘Tcl_Interp’ {aka ‘struct Tcl_Interp’} has no member named ‘result’
    994 |  fprintf(stderr, "Error Message was '%s'\n", sim_interp->result);
        ...
    ```
    同样的问题也会出现在编译`ssim.c`时，这是因为[tcl8.6无法直接接触结构体`Tcl_Interp`](https://stackoverflow.com/questions/66291922/tk-h-looks-for-tcl-h-in-usr-include-but-tcl-h-is-in-usr-include-tcl-i-dont-h)
    - 赋值： `interp->result="string"`变为`Tcl_SetResult(interp, "string", TCL_STATIC)`  例如： `interp->result = "No arguments allowed"`  
    改为`Tcl_SetResult(interp, "No arguments allowed", TCL_STATIC)`

    - 引用： `interp->restlt`变为`Tcl_GetStringResult(sim_interp)`  
    例如： `fprintf(stderr, "Error Message was '%s'\n", sim_interp->result)`  
    改为`fprintf(stderr, "Error Message was '%s'\n", Tcl_GetStringResult(sim_interp))`
    
    所以，我们需要按照上面的标准修改`psim.c, ssim.c`

    这样的兼容问题还有几个。

- `gcc10`兼容问题

    如果你使用的gcc是10代之上（我的是`gcc 11.2.0`），可能会出现这个问题:
    ```
    (cd misc; make all)
    ...
    multiple definition of `lineno'; yas-grammar.o:(.bss+0x0): first defined here
    ```
    此时需要修改`misc/Makefile`:
    ```
    CFLAGS=-Wall -O1 -g
    LCFLAGS=-O1
    New misc/Makefile:
    ```
    加上参数`-fcommon`
    ```
    CFLAGS=-Wall -O1 -g -fcommon
    LCFLAGS=-O1 -fcommon
    ```
    参考这篇文档：[Porting to GCC 10](https://gcc.gnu.org/gcc-10/porting_to.html)

- `glibc`兼容问题
    ```
    (cd pipe; make all GUIMODE=-DHAS_GUI TKLIBS="-L/usr/lib/x86_64-linux-gnu/ -ltk -ltcl" TKINC="-isystem /usr/include/tcl8.6")
    ...
    undefined reference to `matherr'
    ```
    同样，[Glibc不再支持`matherr`](https://stackoverflow.com/questions/52903488/fail-to-build-y86-64-simulator-from-sources)
    ```bash
    linux: archlab/sim$ grep -r matherr .
    ./pipe/psim.c:extern int matherr();
    ./pipe/psim.c:int *tclDummyMathPtr = (int *) matherr;
    ./seq/ssim.c:extern int matherr();
    ./seq/ssim.c:int *tclDummyMathPtr = (int *) matherr;
    ```
    参考回答，将`psim.c, ssim.c`中对应行注释掉即可。

    你以为这就结束啦？还有最后一个问题

- 全局变量定义问题

    ```
    (cd pipe; make all GUIMODE=-DHAS_GUI TKLIBS="-L/usr/lib/x86_64-linux-gnu/ -ltk -ltcl" TKINC="-isystem /usr/include/tcl8.6")
    ...
    /usr/bin/ld: /tmp/ccQDCjMO.o:(.bss+0x0): multiple definition of `mem_wb_state'; /tmp/ccCYohGC.o:(.bss+0x120): first defined here
    /usr/bin/ld: /tmp/ccQDCjMO.o:(.bss+0x8): multiple definition of `ex_mem_state'; /tmp/ccCYohGC.o:(.bss+0x128): first defined here
    /usr/bin/ld: /tmp/ccQDCjMO.o:(.bss+0x10): multiple definition of `id_ex_state'; /tmp/ccCYohGC.o:(.bss+0x130): first defined here
    /usr/bin/ld: /tmp/ccQDCjMO.o:(.bss+0x18): multiple definition of `if_id_state'; /tmp/ccCYohGC.o:(.bss+0x138): first defined here
    /usr/bin/ld: /tmp/ccQDCjMO.o:(.bss+0x20): multiple definition of `pc_state'; /tmp/ccCYohGC.o:(.bss+0x140): first defined here
    ```

    这是因为`pipe/sim.h`和`pipe/psim.c`中全局变量的定义冲突了。  
    在`sim.h`中四个变量前面加上`extern`声明是外部变量即可:
    ```
    /* Provide global access to current states of all pipeline registers */
    pipe_ptr pc_state, if_id_state, id_ex_state, ex_mem_state, mem_wb_state;
    ```
    改为
    ```
    /* Provide global access to current states of all pipeline registers */
    extern pipe_ptr pc_state, if_id_state, id_ex_state, ex_mem_state, mem_wb_state;
    ```
    到这里，只要环境正常，一般来说都能顺利编译出GUI版本的模拟器。大功告成!  

#### 2.3 待解决的问题

-  wsl2-ubuntu配置环境遇到的问题

    在wsl2-ubuntu中，安装`tcl\tk`之后启动`tclsh`会出现下面这个问题：  
    `Tcl wasn't installed properly`!
    ```bash
    linux$ tclsh
    application-specific initialization failed: Can't find a usable init.tcl in the following directories: 
        /usr/local/tcl/lib/tcl8.6 /usr/lib/tcl8.6 /lib/tcl8.6 /usr/library /library /tcl8.6.12/library /tcl8.6.12/library



    This probably means that Tcl wasn't installed properly.

    % 
    ```
    之后我在VM的ubuntu环境下安装了`tcl\tk`，发现两者的文件配置是一样的，貌似单纯只是因为`wsl`不支持GUI工具。这个问题，暂时放弃解决...


## Part A

Part A的三个小程序比较简单，掌握了Y86的基本指令就能很轻松写出来，源文件如下:

1. [`sum.ys`](./sim/misc/sum.ys) 
2. [`rsum.ys`](./sim/misc/rsum.ys) 
3. [`copy.ys`](./sim/misc/copy.ys) 

**一些收获**：
- 为什么初始化寄存器的值为0时要使用`xorq`命令？
    
    之间对这个问题一直一知半解，经过这个小实验的学习，我明白了，答案其实很简单：  
    Y86中，`xorq`的编码是2bytes，而`irmovq`是10bytes。为了优化性能，采用哪一个指令是显而易见的。  
    其他指令集也是同样的道理。


## Part B

### 实验准备

HCL硬件控制语言是HDL(例如Verilog)的一个子集，与Y86类似是用于教学的一种硬件语言。  
![HCL](./HCL.png)

HCL主要用于描述逻辑单元的行为：描述及控制逻辑单元的输入输出(例如Mux4的输入s0, s1, 输出out4)。除此之外的硬件功能在这个实验中是由实验设计者设计的模拟器来实现，我们需要编写的的只有上图中的`pipe-std.hcl`部分的`.hcl`文件，其余部分以链接的方式形成最终的模拟器。  
因此我们只需要明白如下几点就可以顺利的使用他们设计的HCL语言：

1. “宏替换”
```
boolsig name 'C-expr'
intsig name 'C-expr'
```
上面的语句将.hcl中的`name`替换为C语言语句`C-expr`，C-expr不包含`'`和`\n`

2. 定义`C语句`
```
quote   'string'
```
上面的语句定义一个C语言中的语句，string不包含`.`和`\n`

### 实验内容

本实验的目的是在Y86指令集中加入一条`iaddq`使之能让寄存器与常数值相加。

1. 指令构成

![iadd_q](./iaddq.png)

2. 分阶段动作

| Stage | iaddq V, rB |
|-------|-------------|
| Fetch | icode:ifun <- C : 0 <br> rA : rB <- F : rB <br> valC <- V <br> valP <- PC+10|
| Decode | valB <- R[rB] |
| Execute | valE <- valB+valC <br> set CC|
| Memory | | 
| Write bakc | R[rB] <- valE |
| Update PC | PC <- valP |

3. 根据指令动作修改`seq-full.hcl`

源文件：[`seq-full.hcl`](./sim/seq/seq-full.hcl)

4. 回归测试

最后别忘了进行一下回归测试，这用来检测我们修改的`iaddq`有没有引入bug

```bash
cd ../ptest
make SIM=../seq/ssim
make SIM=../seq/ssim TFLAGS=-i
```

## Part C

先上结果：

[`ncopy.ys`](./sim/pipe/ncopy.ys)

最后得分：`50.3/60.0`

我主要做了如下修改：

1. hcl逻辑结构的修改：[`pipe-super.hcl`](./sim/pipe/pipe-super.hcl)
    - pipe结构加入了指令`iaddq`
    - pipe分支预测策略改为`btfnt`模式


2. `ncopy.ys`代码结构的修改：
    - 对循环主体进行8路循环展开
    - 对余下部分进行支路展开，不进行循环
    - 调整了`rmmovq`指令前后的指令顺序，经过调整，我的代码已经完全避开了加载互锁
    - 去除了一些多余指令

写的有点累，分析报告不想写太多了，反正没人看。:mask: