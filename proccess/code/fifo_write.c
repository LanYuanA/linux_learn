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
    //创建有名管道 其权限为0644
    if(mkfifo(pipe_path,0644) != 0)
    {
        perror("mkfifo failed");
        if (errno != 17)
        {
            exit(EXIT_FAILURE);
        }
    }
    //管道创建成功 写入数据
    fd = open(pipe_path,O_WRONLY); //fd指向pipe_path
    if(fd == -1)
    {
        perror("发送端open");
        exit(EXIT_FAILURE);
    }

    char buf[100];
    ssize_t ret = read(STDIN_FILENO,buf,sizeof(buf));
    if(ret == -1)
    {
        perror("发送端read");
        exit(EXIT_FAILURE);
        close(fd);
    }
    ssize_t re = write(fd,buf,strlen(buf));
    if(re == -1)
    {
        perror("发送端write");
        exit(EXIT_FAILURE);
        close(fd);
    }
    printf("发送管道退出，进程终止\n");
    close(fd);

    if(unlink(pipe_path) == -1) {
        perror("fifo_write unlink");
    }
    return 0;
}