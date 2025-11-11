#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

sem_t *full;
sem_t *empty;

int shard_num;

int rand_num()
{
    srand(time(NULL));
    return rand();
}

//生产者线程
void *producer(void *argv)
{
    for(int i = 0 ; i < 5;i++)
    {
        sem_wait(empty); //empty -1
        printf("\n==========> 第 %d 轮数据传输 <=========\n\n", i + 1);
        sleep(1);
        shard_num = rand_num();
        printf("producer has sent data\n");
        sem_post(full);//full +1
    }
}

//消费者线程
void *consumer(void *argv)
{
    for(int i = 0 ; i < 5;i++)
    {
        sem_wait(full); //full -1
        printf("consumer has read data\n");
        printf("the shardnum =%d\n",shard_num);
        sleep(1);
        sem_post(empty);//empty +1
    }
}

int main()
{
    full = malloc(sizeof(sem_t));
    empty = malloc(sizeof(sem_t));

    sem_init(full,0,0);
    sem_init(empty,0,1);

    pthread_t pid_producer;
    pthread_t pid_consume;
    //创建两个线程
    pthread_create(&pid_producer,NULL,producer,NULL);    
    pthread_create(&pid_consume,NULL,consumer,NULL);    

    pthread_join(pid_producer,NULL);
    pthread_join(pid_consume,NULL);
    
    sem_destroy(full);
    sem_destroy(empty);
    return 0;
}