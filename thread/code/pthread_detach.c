#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *task(void *argv)
{
    printf("thread stared\n");
    sleep(2);//模拟线程工作
    printf("thread finished\n");
    return NULL;//有返回值
}

int main()
{
    pthread_t pid__test;
    //创建线程
    pthread_create(&pid__test,NULL,task,NULL);
    //标记为pthread_detach状态
    pthread_detach(pid__test);

    //主线程继续工作
    printf("main thread continue...\n");
    sleep(3);//若主线程在子线程结束前就已经结束 则整个进程退出 子线程被强制退出
    printf("main thread ending...\n");

    return 0;
}