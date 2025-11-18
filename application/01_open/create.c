#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
/*
 * ./create 1.txt
 * argc  = 2
 * argv[0] = "./create"
 * argv[1] = "1.txt"
 */
int main(int argc, char **argv)
{
    int fd; //文件描述符
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }
    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0777);
    if (fd < 0) {
        printf("Error opening file: %s\n", argv[1]); //打开文件错误
        printf("Error number: %d\n", errno); //错误码
        //printf("Error message: %s\n", strerror(errno)); //错误信息
        perror("Error message"); //打开文件错误
    }

    while(1){
        sleep(10);
    }
    close(fd);
    return 0;
}