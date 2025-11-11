#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

typedef struct Result //定义一个结构体 存放指针和长度
{
    char *p;
    int len;
} Result;


//创建两个线程 一个线程退出的时候给另一个线程返回结构体 当接收到结构体的时候 执行函数0
void *white_thread(void *argv)
{
    Result *result = malloc(sizeof(Result));//申请结果结构体的内存
    char code = *((char *)argv);//code存放参数的指针
    //存放回信
    char *ans = malloc(101);
    while (1)
    {
        //从命令行读取数据
        fgets(ans,100,stdin);
        if(ans[0] == code)
        {
            //成功读取到数据
            //释放掉输入数据
            free(ans);
            printf("白线程读到启动命令了\n");
            //定义一个result类型数据作为返回值
            char *whiteAns = strdup("这是白线程的回应\n");
            result->p = whiteAns;
            result->len = strlen(whiteAns);
            //结束进程 把返回值传回
            pthread_exit((void *)result);
        }
        else
        {
            printf("白进程还在运行\n");
        }
    }
}

void *red_thread(void *argv)
{
    Result *result = malloc(sizeof(Result));//申请结果结构体的内存
    char code = *((char *)argv);//code存放参数的指针
    //存放回信
    char *ans = malloc(101);
    while (1)
    {
        //从命令行读取数据
        fgets(ans,100,stdin);
        if(ans[0] == code)
        {
            //成功读取到数据
            //释放掉输入数据
            free(ans);
            printf("红线程读到启动命令了\n");
            //定义一个result类型数据作为返回值
            char *redAns = strdup("这是红线程的回应\n");
            result->p = redAns;
            result->len = strlen(redAns);
            //结束进程 把返回值传回
            pthread_exit((void *)result);
        }
        else
        {
            printf("红进程还在运行\n");
        }
    }
}

int main()
{
    //创建两个线程句柄
    pthread_t pid_white;
    pthread_t pid_red;
    //创建两个参数传给函数
    char white_code = 'w';
    char red_code = 'r';
    //创建两个result类型的结果保存返回值
    Result *white_result = NULL;
    Result *red_result = NULL;
    //创建两个线程
    pthread_create(&pid_white,NULL,white_thread,&white_code);
    pthread_create(&pid_red,NULL,red_thread,&red_code);
    //获取线程结果
    pthread_join(pid_white,&white_result);
    printf("这是对白线程的测试:%s",white_result->p);
    //释放内存
    free(white_result->p);
    free(white_result);

    pthread_join(pid_red,&red_result);
    printf("这是对红线程的测试:%s",red_result->p);
    //释放内存
    free(red_result->p);
    free(red_result);
    
    return 0;
}