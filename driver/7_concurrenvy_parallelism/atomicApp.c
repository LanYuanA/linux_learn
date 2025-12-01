#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/***************************************************************
文件名 : atomicApp.c
作者 : lanyuana
版本 : V1.0
描述 : 原子操作测试app 使用原子变量来实现只允许一个应用程序使用LED。
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

#define LEDOFF 0/*关灯*/
#define LEDON 1/*开灯*/

int main(int argc ,char *argv[])
{
    int fd,retvalue,cnt;
    char *filename;
    if(argc !=3)
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

    unsigned char databuf[1];
    databuf[0] = atoi(argv[2]);/*存放要执行的指令 打开或关闭*/
    /*向/dev/led写数据*/
    retvalue = write(fd,databuf,sizeof(databuf));
    if(retvalue<0)
    {
        perror("write");
        close(fd);
        exit(EXIT_FAILURE);        
    }
    /* 模拟占用25s led */
    while (1)
    {
        /* code */
        sleep(5);
        cnt++;
        //printf("App running times:%d\r\n",cnt);
        if(cnt>=5)break;
    }
    printf("App running finished!");
    retvalue = close(fd);
    if(retvalue<0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        exit(EXIT_FAILURE);        
    }
    return 0;
}