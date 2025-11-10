#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


int main(int argc, char const *argv[])
{
    //创建消息队列
    struct mq_attr attr;
    //设置有用的参数 表示队列的容量
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    //其他参数 在创建队列的时候用不到
    attr.mq_curmsgs = 0; //当前队列的消息数量
    attr.mq_flags = 0; //标记

    char *name = "/father_son_mq";
    mqd_t mqdes = mq_open(name,O_RDWR | O_CREAT,0644,&attr);

    if(mqdes == (mqd_t)-1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    //创建父子进程
    pid_t pid = fork();
    if(pid <0 )
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        //子进程要执行的代码 从消息队列里面读取信息
        char receive_buf[100];
        struct  timespec  time_info;
        for(int i=0;i<10;i++)
        {
            //清空buf
            memset(receive_buf,0,100);//写100个0进缓冲区
            //printf("子进程第%d次接收信息\n",(i+1));
            //获取当前的具体时间
            clock_gettime(0,&time_info);
            time_info.tv_sec +=15;
            //接受信息 打印到控制台
            if(mq_timedreceive(mqdes,receive_buf,100,0,&time_info) == -1)
            {
                perror("mq_timedreceive");
            }
            write(STDOUT_FILENO,receive_buf,strlen(receive_buf));
        }
    }
    else
    {
        //父进程要执行的代码 往消息队列里面写入信息
        char send_buf[100];
        struct  timespec  time_info;
        for(int i=0;i<10;i++)
        {
            //清空buf
            memset(send_buf,0,100);//写100个0进缓冲区
            sprintf(send_buf,"父进程第%d次发送信息\n",(i+1));
            //获取当前的具体时间
            clock_gettime(0,&time_info);
            time_info.tv_sec +=5;
            //发送信息
            if(mq_timedsend(mqdes,send_buf,strlen(send_buf),0,&time_info) == -1)
            {
                perror("mq_timedsend");
            }
            printf("父进程发送一条信息 休息1s\n");
            sleep(1);
        }
    }
    //父子进程都要进行的代码
    close(mqdes);//释放消息队列的引用
    if(pid > 0)//父进程释放消息队列 只需要引用一次
    {
        int ret = mq_unlink(name);
        if(ret < 0)
        {
            perror("mq_unlink");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}