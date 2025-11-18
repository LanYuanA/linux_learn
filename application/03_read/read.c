#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
/*
 * ./read 1.txt  
 * argc  = 2  
 * argv[0] = "./read"
 * argv[1] = "1.txt"
 */
int main(int argc, char **argv)
{
    int fd; //文件描述符
    int len;
    unsigned char buf[100];
    if (argc !=2 ) {
        printf("Usage: %s <file> \n", argv[0]);
        return -1;
    }
    fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("Error opening file: %s\n", argv[1]); //打开文件错误
        printf("Error number: %d\n", errno); //错误码
        //printf("Error message: %s\n", strerror(errno)); //错误信息
        perror("Error message"); //打开文件错误
    }
    printf("fd = %d\n", fd);

    /*读文件 打印*/
    while(1)
    {
        len = read(fd, buf, sizeof(buf)-1);//read之后 第一次read后第二次read从第一次结束的末尾进行
        if(len < 0)
        {
            perror("Read");
            close(fd);
            return -1;

        }
        else if (len == 0){
            break;
        }
        else
        {
            /* buf[0] buf[1] buf[2] ... buf[len-1]为读出的数据
             *  buf[len]为结束符
             */
            buf[len] = '\0';
            printf("%s", buf);
        }
    }
    close(fd);
    return 0;
}