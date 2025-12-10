#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "poll.h"
#include "sys/select.h"
#include "sys/time.h"
#include "linux/ioctl.h"
#include "signal.h"
/***************************************************************
文件名 : asyncnotiApp.c
作者 : lanyuana
版本 : V1.0
描述 : 异步通知测试 APP
其他 : 无
使用方法 ：./asyncnotiApp /dev/asyncnoti 打开测试 App
日志 : 2025/12/02 lanyuana创建
***************************************************************/

static int fd =0;//文件描述符

/*
    SIGIO信号处理函数
    @param - signum : 信号值
    @return : 无
*/
static void sigio_signal_func(int signum)
{
    int err=0;
    unsigned int keyvalue = 0;
    err = read(fd,&keyvalue,sizeof(keyvalue));//触发信号就从内核空间读一次值
    if(err < 0)
    {
        /*读取错误*/
    }
    else
    {
        printf("sigio signal!key value = %d\r\n",keyvalue);
    }
}



int main(int argc ,char *argv[])
{
    int fd,ret;
    int flags;
    char *filename;

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
    /*注册信号signal*/
    signal(SIGIO,sigio_signal_func);
    fcntl(fd, F_SETOWN, getpid()); /* 将当前进程的进程号告诉给内核 */
    flags = fcntl(fd, F_GETFL); /* 获取当前的进程状态 */
    fcntl(fd, F_SETFL, flags | FASYNC); /* 开启当前进程异步通知功能 */

    while(1)
    {
        sleep(2);
    }
    ret = close(fd);
    if(ret<0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        exit(EXIT_FAILURE);        
    }
    return ret;
}