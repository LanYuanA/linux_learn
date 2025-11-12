#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char *path = "/usr/bin/ping";
    char *argv[] = {"ping","-c","10","www.baidu.com",NULL};
    char *env[] = {NULL};
    if(execve(path,argv,env) < 0)
    {
        perror("execve");
    }  
    return 0;
}