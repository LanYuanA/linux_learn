#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define THREAD_COUNT 20000
/*该程序用于测试创建20000个线程 每次线程结束都返回值+1 结果是否等于20000*/
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;//初始化互斥锁

void *add_test(void *argv)
{
    int *p = argv;
    //累加前加锁 其他线程会被阻塞
    pthread_mutex_lock(&counter_mutex);
    (*p)++;
    //累加后解锁
    pthread_mutex_unlock(&counter_mutex);
    return 0;
}

int main()
{
    pthread_t pid[THREAD_COUNT];
    int num = 0;
    for(int i=0;i<THREAD_COUNT;i++)
    {
        //创建线程
        pthread_create(&pid[i],NULL,add_test,&num);
    }

    //等待所有进程结束
    for(int i=0;i<THREAD_COUNT;i++)
    {
        pthread_join(pid[i],NULL);
    }

    printf("最后结果是%d\n",num);
    return 0;
}