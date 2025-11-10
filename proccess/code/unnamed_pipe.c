#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    /* 父进程将argv[1]写入匿名管道 子进程读取并输出到控制台*/
    //pipefd[0]是读端 pipefd[1]是写端
    int pipefd[2];
    char buf;
    int status;
    if(argc != 2)
    {
        perror("请填写要传递的信息\n");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipefd) == -1) //这边开始创建管道
    {
        perror("创建管道失败\n");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {   //子进程要执行的代码 读数据并输出到控制台
        //关闭管道写端
        close(pipefd[1]);
        //从管道里读数据存放到缓冲区
        printf("子进程%d接受到数据\n",getpid());
        ssize_t ret = read(pipefd[0],&buf,strlen(&buf));
        if(ret >= 0)
        {
            printf("成功读取到数据 下面开始打印接受到的数据\n");
        }
        else if(ret == -1)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        ssize_t re = write(STDOUT_FILENO,&buf,strlen(&buf));
        if(re < 0)
        {
            perror("输出到控制台\n");
            close(pipefd[0]);
            exit(EXIT_FAILURE);
        }
        write(STDOUT_FILENO,"\n",1);
        printf("数据已全部输出到控制台\n");
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }
    else
    {   //父进程要执行的代码 往管道里写数据 要关闭读端
        close(pipefd[0]);
        //写传入的参数到管道的写端 argv[1]是要发送的数据
        printf("父进程%d发送数据\n",getpid());
        write(pipefd[1],argv[1],strlen(argv[1]));
        //写完关闭管道写端
        close(pipefd[1]);
        //等待子进程结束
        waitpid(pid,&status,WUNTRACED);
    }
    return 0;
}