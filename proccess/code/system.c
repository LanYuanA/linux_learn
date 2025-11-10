#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv[])
{
    /*
        根据传入的命令启动一个进程
        参数：传入可执行的Shell命令
        return:成功返回0 不支持shell返回-1 失败返回非零
    */
    int ret = system("ping -c  10 www.baidu.com");
    if(ret != 0)
    {
        printf("执行命令失败");
        return 1;
    }
    return 0;
}