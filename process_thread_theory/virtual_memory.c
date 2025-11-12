#include <stdio.h>
#include <unistd.h>

int main()
{
    int val = 123;

    __pid_t pid;

    pid = fork();

    if(pid > 0)
    {
        sleep(1);
        printf("父进程中val的值是%d 其地址是%p\n",val,&val);
    }
    else if(pid == 0)
    {
        val = 321;
        printf("子进程中val的值是%d 其地址是%p\n",val,&val);
    }
    return 0 ;
}