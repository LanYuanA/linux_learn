#include "stdio.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include <signal.h>   // 新增：自动回收子进程
#include <errno.h>    // 新增：检查 read 错误
/***************************************************************
文件名 : keyApp.c
作者 : lanyuana
版本 : V1.0
描述 : 按键测试app 按键控制led开关
其他 : 无
日志 : 初版 V1.0 2025/11/27 lanyuana创建
***************************************************************/

#define KEY0VALUE 0XF0 /*按键值*/
#define INVAKEY 0X00    /*无效值*/

int main(int argc ,char *argv[])
{
    int fd,retvalue;
    char *filename;
    unsigned char keyvalue; 
    int flag= 1;

    if(argc !=2)
    {
        perror("input error");
        exit(EXIT_FAILURE);

    }
    filename = argv[1];//设备文件
    /*打开key驱动：只读即可，避免不必要权限*/
    fd = open(filename,O_RDONLY);
    if(fd<0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }
    /* 自动回收子进程，避免僵尸 */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sa, NULL);
    
    /*循环读取按键值数据*/
    while(1)
    {
        retvalue = read(fd,&keyvalue,sizeof(keyvalue));
        if (retvalue <0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        /* 仅在非按下 -> 按下 边沿触发一次 */
        if(keyvalue == KEY0VALUE )
        {
            printf("get key!\r\n");
            flag ^=1;
            if(flag == 0)   printf("LED ON!\r\n");
            else printf("LED OFF!\r\n");
            pid_t pid = fork();
            if(pid < 0 )
            {
                perror("fork");
            }
            else if(pid == 0)
            {
                //子进程
                close(fd);

                const char *path = "/lib/modules/4.1.15/testApp";
                char *const args[] = { "./testApp","/dev/gpioled",flag ? "0" : "1", NULL };
                char *env[] = {NULL};
                execve(path, args, env);
                /* execve失败才会到这里 */
                perror("execve");
                _exit(127);
            }
            /*父进程什么都不做 继续读取按键值*/
        }
        while (waitpid(-1, NULL, WNOHANG) > 0) { }
    }
    retvalue = close(fd);
    if(retvalue < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
    return 0;
}