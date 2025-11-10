#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main()
{
    printf("返回调用进程的pid:%d\n",getpid());
    pid_t pid = fork();
    if (pid < 0)
    {
        //创建新进程失败
        printf("新进程创建失败\n");
        return 1;
    }
    else if(pid == 0)
    {
        printf("新进程%d创建成功,他是由父进程%d创建的\n",getpid(),getppid());
    }
    else
    {
        printf("父进程%d继续运行,他创建了子进程%d\n",getpid(),pid);        
    }
    return 0;
}