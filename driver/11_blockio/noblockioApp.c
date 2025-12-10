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
/***************************************************************
文件名 : noblockioApp.c
作者 : lanyuana
版本 : V1.0
描述 : 非阻塞访问测试 APP
其他 : 无
日志 : 2025/12/02 lanyuana创建
***************************************************************/


int main(int argc ,char *argv[])
{
    int fd,ret,cnt;
    int flag;
    char *filename;
    unsigned char data;
    struct pollfd fds;//poll
    fd_set readfds;//select
    struct timeval timeout;
    if(argc != 2)
    {
        perror("input error");
        exit(EXIT_FAILURE);

    }
    filename = argv[1];//设备文件
    /*打开led驱动 非阻塞*/
    fd = open(filename,O_RDWR | O_NONBLOCK);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    fds.fd = fd;
    fds.events = POLLIN | POLLRDNORM;
#if 0
    while(1)
    {
        ret = poll(&fds,1,500);
        if(ret)//数据有效
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
        else if(ret == 0)//超时
        {
            printf("timeout!\r\n");
        }
        else if (ret < 0 )
        {
            printf("error!\r\n");
        }
    }
#endif

#if 1
    while(1)
    {
        FD_ZERO(&readfds);//将所有位清零
        FD_SET(fd,&readfds);//加入描述符集
        /*构造超时时间*/
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;/*500ms*/
        ret = select(fd+1,&readfds,NULL,NULL,&timeout);
        switch (ret)
        {
        case 0://超时
            /* code */
            printf("timeout!\r\n");
            break;
        case -1://错误
            printf("error!\r\n");
            break;
        default://可以读取数据
            if(FD_ISSET(fd,&readfds))//属于描述符集
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
            break;
        }
    }
#endif
    ret = close(fd);
    if(ret<0)
    {
        printf("file %s close failed!\r\n", argv[1]);
        exit(EXIT_FAILURE);        
    }
    return ret;
}