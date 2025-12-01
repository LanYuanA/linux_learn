#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
/***************************************************************
文件名 : imx6uirq.c
作者 : lanyuana
版本 : V1.0
描述 : 原子操作测试app 使用原子变量来实现只允许一个应用程序使用LED。
其他 : 无
日志 : 初版 V1.0 2025/12/01 lanyuana创建
***************************************************************/


int main(int argc ,char *argv[])
{
    int fd,ret,cnt;
    char *filename;
    unsigned char data;
    if(argc != 2)
    {
        perror("input error");
        exit(EXIT_FAILURE);

    }
    filename = argv[1];//设备文件
    /*打开led驱动*/
    fd = open(filename,O_RDWR);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        ret = read(fd,&data,sizeof(data));
        if(ret < 0 )
        {
            /*数据读取错误*/
        }
        else
        {
            /*成功读取到数据*/
            if(data)
            {
                printf("data get!:%#X\r\n",data);
            }
        }
    }

    ret = close(fd);
    if(ret<0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        exit(EXIT_FAILURE);        
    }
    return 0;
}