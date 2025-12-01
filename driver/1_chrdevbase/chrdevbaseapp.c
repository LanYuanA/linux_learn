#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"

/***************************************************************
Copyright © ALIENTEK Co., Ltd. 1998-2029. All rights reserved.
文件名 : chrdevbaseapp.c
作者 : lanyuana
版本 : V1.0
描述 : chrdevbase 驱动测试app。
其他 : 使用方式：./chrdevbaseapp filename <1>|<2>
        <1> 读数据并打印在控制台
        <2> 发送userdata到驱动文件
日志 : 初版 V1.0 2025/11/17 lanyuana创建
***************************************************************/

static char userdata[]={"user data!"};

int main(int argc,char *argv[])
{
    int fd,retvalue;
    char *filename;
    char readbuf[100],writebuf[100];
    if(argc != 3)
    {
        perror("输入参数错误");
        exit(EXIT_FAILURE);
    }
    filename = argv[1];
    /*打开驱动文件*/
    fd = open(filename,O_RDWR);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    /*从驱动文件中读取数据*/
    if(atoi(argv[2]) == 1)
    {
        retvalue = read(fd,readbuf,50);
        if(retvalue<0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        else
        {
            /*读取成功打印成功数据*/
            printf("read data:%s\r\n",readbuf);
            if(retvalue<0)
            {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
    if(atoi(argv[2]) == 2)
    {
        /*写设备驱动写数据*/
        memcpy(writebuf,userdata,sizeof(userdata));
        retvalue = write(fd,writebuf,50);
        if(retvalue<0)
        {
            perror("write to chrdev");
            exit(EXIT_FAILURE);
        }
    }
    /*关闭设备*/
    retvalue = close(fd);
    if(retvalue<0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;    
}