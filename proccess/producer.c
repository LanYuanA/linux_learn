#include <time.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
//生产者 往消息队列里面写入数据
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
    mqd_t mqdes = mq_open(name,O_WRONLY | O_CREAT,0666,&attr);
    if(mqdes == -1)
    {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
    char send_buf[100];
    struct timespec time_info;
    while(1){
        //清空缓冲区
        memset(send_buf,0,100);
        //从命令行获取数据
        ssize_t read_cnt = read(STDIN_FILENO,send_buf,100);
        if(read_cnt <0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        clock_gettime(0,&time_info);
        time_info.tv_sec += 5;
        //退出
        if(read_cnt == 0)
        {
            printf("Received EOF,Exit...\n");
            char eof = EOF;
            if(mq_timedsend(mqdes,&eof,1,0,&time_info) == -1)
            {
                perror("mq_timedsend");
            }
            break;
        }
        //正常发送数据
        if(read_cnt>0)
        {
            if(mq_timedsend(mqdes,send_buf,strlen(send_buf),0,&time_info) == -1)
            {
                perror("mq_timedsend");
            }
            printf("从命令行接收到数据,已发送到消费者端\n");
        }
    }
    //发送结束后关闭连接
    close(mqdes);
    return 0;
}
