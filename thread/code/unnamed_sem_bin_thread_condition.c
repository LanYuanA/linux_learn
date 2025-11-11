#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>

int shard_num = 0;
sem_t unnamed_sem; 

void *add_num(void *argv)
{
    sem_wait(&unnamed_sem);
    int tmp = shard_num +1;
    shard_num = tmp;
    sem_post(&unnamed_sem);
}

int main()
{
    sem_init(&unnamed_sem,0,1);
    pthread_t pid[10000];
    for(int i = 0;i < 10000; i++)
    {
        pthread_create(&pid[i],NULL,add_num,NULL);
    }

    for(int i = 0;i < 10000; i++)
    {
        pthread_join(pid[i],NULL);
    }
    printf("最后的结果是：%d\n",shard_num);
    sem_destroy(&unnamed_sem);
    return 0;
}