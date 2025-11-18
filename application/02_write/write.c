#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
/*
 * ./write 1.txt  str1 str2
 * argc  = 2  
 * argv[0] = "./write"
 * argv[1] = "1.txt"
 */
int main(int argc, char **argv)
{
    int fd; //文件描述符
    int i;
    int len;
    if (argc <3 ) {
        printf("Usage: %s <file> <str1> <str2> ...\n", argv[0]);
        return -1;
    }
    fd = open(argv[1], O_RDWR | O_CREAT, 0777);//可读读写 创建
    if (fd < 0) {
        printf("Error opening file: %s\n", argv[1]); //打开文件错误
        printf("Error number: %d\n", errno); //错误码
        //printf("Error message: %s\n", strerror(errno)); //错误信息
        perror("Error message"); //打开文件错误
    }
    printf("fd = %d\n", fd);

    printf("lseek to offset 3 from file head\n");
    lseek(fd, 3, SEEK_SET);
    write(fd, "abc", 3);


/*    for(i = 2; i < argc; i++) 
    {
        len = write(fd, argv[i], strlen(argv[i]));
        if(len != strlen(argv[i]))
        {
            perror("Write");
            break;
        }
        write(fd,"\r\n",2);
    }
    while(1)
    {
        pause();
    }
    close(fd);
    return 0;
*/
}