#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

/*  实现父进程创建一个子进程跳转到erlou 而自己保持不变

*/

int main(int argc, char const *argv[])
{
    char *name="父进程";
    printf("我是%s,%d,现在在父进程\n",name,getpid());
    pid_t pid = fork();
    if(pid < 0 )
    {
        printf("子进程创建失败\n");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        //子进程要执行的代码
        char *name1 = "子进程";
        char *args[]={"/home/lanyuana/code/proccess/erloublock",name1 ,NULL};
        char *envs[]={"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
        int ret = execve(args[0],args,envs);
        if(ret < 0)
        {
            printf("子进程跳转失败\n");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    else
    {
        //父进程要执行的代码
        //sleep(1);
        printf("我是%s,%d,我仍在父进程\n",name,getpid());
        //char bye = fgetc(stdin);
    }
    return 0;
}