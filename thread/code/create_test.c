#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_LEN 1024

char *buf;
/**
 * @brief 线程input_thread读取标准输入的数据，放入缓存区
 * 
 * @param argv 不需要输入参数，所以固定为NULL
 * @return void* 也不需要返回值
 */
void *input_thread(void *argv)
{
    int i=0;
    while(1)
    {
        //从标准输入读取一个字符
        char c = fgetc(stdin);
        //若不是换行或者0 则写入缓冲区
        if(c && c != '\n')
        {
            buf[i++] = c;
            //缓冲区索引溢出 i归零
            if(i >= BUF_LEN)
            {
                i = 0;
            }
        }
    }
}

/**
 * @brief 线程output_thread从缓存区读取数据写到标准输出，每个字符换行
 * 
 * @param argv 不需要输入参数，所以固定为NULL
 * @return void* 也不需要返回值
 */
void *output_thread(void *argv)
{
    //从缓冲区读数据并打印到stdout上
    int i = 0;
    while(1)
    {
        if(buf[i])
        {
            fputc(buf[i],stdout);
            fputc('\n',stdout);
            
            //清除读取完毕的字节
            buf[i] = 0;
            i++;
            if(i > BUF_LEN)
            {
                i = 0;
            }
        }
        else
        {
            sleep(1);
        }
    }
}

int main()
{
    pthread_t pid_input_thread;
    pthread_t pid_output_thread;

    //分配缓存
    buf = malloc(BUF_LEN);
    //初始化缓冲区
    for(int i = 0;i < BUF_LEN;i++)
    {
        buf[i] = 0;
    }
    //创建读取线程
    pthread_create(&pid_input_thread,NULL,input_thread,NULL);
    //创建写出线程
    pthread_create(&pid_output_thread,NULL,output_thread,NULL);
    //等待线程结束
    pthread_join(pid_input_thread,NULL);
    pthread_join(pid_output_thread,NULL);

    //释放缓冲区
    free(buf);
    return 0;
}