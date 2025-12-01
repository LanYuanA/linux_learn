#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "linux/ioctl.h"
/***************************************************************
文件名 : timerApp.c
作者 : lanyuana
版本 : V1.0
描述 : 原子操作测试app 使用原子变量来实现只允许一个应用程序使用LED。
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

/* 命令值 */
#define CLOSE_CMD (_IO(0XEF, 0x1)) /* 关闭定时器 */
#define OPEN_CMD (_IO(0XEF, 0x2)) /* 打开定时器 */
#define SETPERIOD_CMD (_IO(0XEF, 0x3)) /* 设置定时器周期命令 */
#define LEDOFF 0/*关灯*/
#define LEDON 1/*开灯*/

int main(int argc ,char *argv[])
{
    int fd,ret,cnt;
    char *filename;
    unsigned int cmd;
    unsigned int arg;
    unsigned char str[100];
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
        printf("input cmd:\r\n");
        printf("1:close 2:open 3:remod\r\n");

        ret = scanf("%d",&cmd);
        if(ret != 1)
        {
            printf("input again!\r\n");
        }
        if(cmd == 1)    cmd =CLOSE_CMD;
        else if(cmd ==2)    cmd =  OPEN_CMD;
        else if(cmd == 3)
        {
            cmd = SETPERIOD_CMD;
            printf("input timer period:\r\n");
            ret = scanf("%d",&arg);
        }
        ioctl(fd,cmd,arg);   /*调用参数传给驱动 从用户态到内核态*/
    }

    ret = close(fd);
    if(ret<0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        exit(EXIT_FAILURE);        
    }
    return 0;
}