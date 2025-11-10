#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//消费者 从消息队列里面读数据并打印
int main()
{
    struct  mq_attr attr;
    char *name = "/my_mq";
    //设置消息队列参数
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 100;
    //创建消息队列时两个无用参数
    attr.mq_flags = 0;
    attr.mq_curmsgs =0;

    //创建消息队列描述符
    mqd_t mqdes = mq_open(name,O_RDONLY | O_CREAT,0666,&attr);
    if(mqdes == -1)
    {
        perror("mq_open");
    }
    char read_buf[100];
    struct timespec time_info;
    while(1)
    {
        //清空缓冲区
        memset(read_buf,0,100);
        clock_gettime(0,&time_info);
        //获取1天后的timespec结构对象 目的是测试期间使得消费者一直等待生产者发送的数据
        time_info.tv_sec += 86400;
        //从消息队列中获取信息
        if(mq_timedreceive(mqdes,read_buf,100,NULL,&time_info) == -1)
        {
            perror("mq_timedreceive");
        }
        if(read_buf[0] == EOF)
        {
            printf("接受到生成者发送的终止信号 准备退出...\n");
            break;            
        }
        printf("接收到来自于生产者的数据\n%s", read_buf);
    }
        //发送结束后关闭连接
        close(mqdes);
        mq_unlink(name);
}