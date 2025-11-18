#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

int main()
{
    FILE *file = fopen("./testfile.txt","w");
    if(file == NULL )
    {
        perror("fopen");
        return 1;
    }
    //设置文件为无缓冲模式
    if(setvbuf(file,NULL,_IONBF,0) != 0)
    {
        perror("setvbuf");
        return 1;
    }

    fprintf(file,"hello");

    char *argv[] = {"ping","-c","1","www.baidu.com",NULL};
    char *env[] = {NULL};
    execve("/usr/bin/ping",argv,env);

    return 0;
}