#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char *argv[])
{
    /*exec系列函数  父进程跳转进入一个新进程推荐使用execve
    char *__path: 需要执行程序的完整路径名
    char *const __argv[]: 指向字符串数组的指针 需要传入多个参数
        (1) 需要执行的程序命令(同*__path)
        (2) 执行程序需要传入的参数
        (3) 最后一个参数必须是NULL
    char *const __envp[]: 指向字符串数组的指针 需要传入多个环境变量参数
        (1) 环境变量参数 固定格式 key=value
        (2) 最后一个参数必须是NULL
    return: 成功就回不来了 下面的代码都没有意义
            失败返回-1
    int execve (const char *__path, char *const __argv[], char *const __envp[]) 
    */
    char *name = "进程1";
    printf("我是%s,进程号pid是%d 现在在execve程序\n", name ,getpid());
    char *args[]={"/home/lanyuana/code/proccess/erlou",name ,NULL};
    char *envs[]={"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
    int re = execve(args[0],args,envs);
    if (re == -1)
    {
        printf("跳转二楼失败\n");
        return -1;
    }
    return 0;
}