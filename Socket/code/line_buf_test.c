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
    //char *buf = malloc(100);
    //设置文件为行缓冲模式
    if(setvbuf(file,NULL,_IOFBF,0) != 0)
    {
        perror("setvbuf");
        return 1;
    }

    fprintf(file,"hello\n");
    fflush(file);
    char *argv[] = {"ping","-c","1","www.baidu.com",NULL};
    char *env[] = {NULL};
    execve("/usr/bin/ping",argv,env);
    //free(buf);
    return 0;
}