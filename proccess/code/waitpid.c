#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
/*  实现父进程创建一个子进程跳转到erlou 而自己保持不变

*/

int main(int argc, char const *argv[])
{
    int process_status;
    char *name="父进程";
    printf("我是%s,%d,现在在父进程\n",name,getpid());
    pid_t pid = fork();
    if(pid < 0 )
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {   
        //子进程要执行的代码
        //char *name1 = "子进程";
        char *args[]={"/usr/bin/ping","-c","50","www.baidu.com",NULL};
        char *envs[]={"PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin",NULL};
        printf("子进程%d ping50次www.baidu.com\n",getpid());
        int ret = execve(args[0],args,envs);
        if(ret < 0)
        {
            perror("execve");
            exit(EXIT_FAILURE);
        }
        return 0;
    }
    else
    {
        //父进程要执行的代码
        printf("我是%s,%d,我在等待子进程%d结束\n",name,getpid(),pid);
        waitpid(pid,&process_status,WUNTRACED);
    }
    printf("父进程等待子进程结束完毕\n");
    return 0;
}