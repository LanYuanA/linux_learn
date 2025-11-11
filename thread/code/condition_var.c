#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5
static int buffer[BUFFER_SIZE];
static int cnt = 0 ;
//初始化锁
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//初始化条件变量
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//生产者线程
void *producer(void *argv)
{
    int item = 1;
    while(1)
    {
        //获得锁 得不到就等待
        pthread_mutex_lock(&mutex);
        //获得锁之后进行操作

        //若缓冲区已满 则等待消费者读取
        if(cnt == BUFFER_SIZE)
        {
            //暂停线程 等待唤醒
            pthread_cond_wait(&cond,&mutex);
        }
        //缓冲区未满 继续写入
        buffer[cnt] = item;
        cnt++;
        item++;
        if(item == 10)
        {
            item = 1;
        }
        printf("生产者写入了一个数据%d\n",buffer[cnt-1]);
        //通知消费者 读取数据
        pthread_cond_signal(&cond);
        //解锁互斥锁
        pthread_mutex_unlock(&mutex);
    }
}

//消费者线程
void *consumer(void *argv)
{
    int data = 0;
    while(1)
    {
        //获得锁 得不到就等待
        pthread_mutex_lock(&mutex);
        //获得锁之后进行操作
        sleep(1);
        //若缓冲区是空 则等待生产者写入
        if(cnt == 0)
        {
            //暂停线程 等待唤醒
            pthread_cond_wait(&cond,&mutex);
        }
        //缓冲区不是空 继续读取
        data = buffer[cnt];
        cnt --;
        printf("消费者读取了一个数据%d\n",data);
        //通知生产者 写入数据
        pthread_cond_signal(&cond);
        //解锁互斥锁
        pthread_mutex_unlock(&mutex);
    }
}

int main()
{
    pthread_t pid_producer;
    pthread_t pid_consume;
    //创建两个线程
    pthread_create(&pid_producer,NULL,producer,NULL);    
    pthread_create(&pid_consume,NULL,consumer,NULL);    

    pthread_join(pid_producer,NULL);
    pthread_join(pid_consume,NULL);
    
    return 0;
}