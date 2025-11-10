#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
int main()
{
    int fd;
    char *pipe_path = "/tmp/myfifo";
    //打开有名管道 用于读取
    fd = open(pipe_path,O_RDONLY); //fd指向pipe_path
    if(fd == -1)
    {
        perror("接收端open");
        exit(EXIT_FAILURE);
    }

    char buf[100];
    ssize_t ret = read(fd,buf,sizeof(buf));
    if(ret == -1)
    {
        perror("接收端read");
        exit(EXIT_FAILURE);
        close(fd);
    }
    ssize_t re = write(STDOUT_FILENO,buf,strlen(buf));
    if(re == -1)
    {
        perror("接收端write");
        exit(EXIT_FAILURE);
        close(fd);
    }

    printf("已经写入到控制台\n");
    close(fd);
    return 0;
}