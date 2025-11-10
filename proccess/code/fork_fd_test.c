#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
int main(int argc, char const *argv[])
{
    //fork之前
    //打开一个文件
    int fd;
    fd = open("io.txt" , O_CREAT | O_WRONLY | O_APPEND,0644);
    if(fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    //缓存区存放写出的数据
    char buf[1024];
    pid_t pid = fork();
    if(pid < 0 )
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        //子进程代码
        strcpy(buf,"这是子进程写入的数据\n");
    }
    else
    {
        //父进程代码
        sleep(1);
        strcpy(buf,"这是父进程写入的数据\n");
    }

    //父子进程都要执行的代码
    ssize_t bytes_write = write(fd,buf,strlen(buf));
    if(bytes_write == -1)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("写入数据成功\n");
    }
    //使用结束后关闭
    close(fd);
    if(pid == 0)
    {
        //子进程要执行的代码
        printf("子进程写入完毕 释放了文件描述符\n");
    }
    else
    {
        //父进程要执行的代码
        printf("父进程写入完毕 释放了文件描述符\n");
    }
    return 0;
}