#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc,char **argv)
{
    int fd;
    /*读数据 io.txt*/
    fd = open("io.txt",O_RDONLY);
    if(fd == -1)
    {
        perror("open");
        return -1;
    }
    else
    {
        printf("open this file successfully\n");
    }
    /*将数据写入标准输出*/
    char buf[1000];
    int ret = read(fd,buf,sizeof(buf));
    if(ret == -1)
    {
        perror("read\n");
        close(fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("read this file successfully\n");
    }
    write(STDOUT_FILENO,buf,ret);
    /*关闭文件*/
    close(fd);
    return 0;
}