#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名 : ledApp.c
作者 : lanyuana
版本 : V1.0
描述 : beepApp。
其他 : 使用方法 ./beepApp /dev/beep 0 关蜂鸣器
               ./beepApp /dev/beep 1 开蜂鸣器
日志 : 初版 V1.0 2025/11/22 lanyuana创建
***************************************************************/

#define BEEPOFF 0/*关蜂鸣器*/
#define BEEPON 1/*开蜂鸣器*/

int main(int argc ,char *argv[])
{
    int fd,retvalue;
    char *filename;
    if(argc !=3)
    {
        perror("input error");
        exit(EXIT_FAILURE);

    }
    filename = argv[1];//设备文件
    /*打开beep驱动*/
    fd = open(filename,O_RDWR);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    unsigned char databuf[1];
    databuf[0] = atoi(argv[2]);/*存放要执行的指令 打开或关闭*/
    /*向/dev/beep写数据*/
    retvalue = write(fd,databuf,1);
    if(retvalue<0)
    {
        perror("write");
        exit(EXIT_FAILURE);        
    }
    /*从/dev/led读数据*/
    retvalue = read(fd,databuf,1);
    //printf("data:%d",databuf[0]);
    if(databuf[0] == BEEPOFF)
    {
        printf("BEEPOFF!\n");
    }
    else if(databuf[0] == BEEPON)
    {
        printf("BEEPON!\n");
    }
    retvalue = close(fd);
    if(retvalue<0)
    {
        perror("close");
        exit(EXIT_FAILURE);        
    }    
    return 0;
}