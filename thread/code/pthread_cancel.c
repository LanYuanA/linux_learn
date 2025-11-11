#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *task(void *arg)
{
    printf("Thread started\n");//默认线程状态是join deferred
    //若设置退出模式为异步 则一般是立刻退出 不保存之前状态
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
    sleep(1);
    //模拟工作
    printf("Thread working...\n");
    pthread_testcancel();//取消点函数
    printf("after cancelled\n");
    return NULL;    
}

int main()
{
    pthread_t pid__test;
    //创建线程
    pthread_create(&pid__test,NULL,task,NULL);

    //取消线程
    int re = pthread_cancel(pid__test);
    if(re != 0)
    {
        perror("pthread_cancel");
    }

    //等待子线程终止并获取退出状态
    char *res;
    pthread_join(pid__test,(void **)&res);

    //检查是否被取消
    if(res == PTHREAD_CANCELED)
    {
        printf("thread cancelled successfully\n");
    }
    else
    {
        printf("Thread was not canceled, exit code: %ld\n", (long)res);
    }
    return 0;
}