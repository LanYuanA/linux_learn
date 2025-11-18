# 缓冲区

```c
#include <stdio.h>
/**
 * @brief 设置文件流的缓冲模式
 * 
 * @param stream 要设置的文件流，可以是输入缓冲流也可以是输出缓冲流
 * @param buf 指向缓冲区的指针，这个参数为NULL则自动分配缓冲区。
 * @param mode 缓冲区模式
 *        _IOFBF：全缓冲，数据会存储在缓冲区中直到缓冲区满。
 *        _IOLBF：行缓冲，数据会存储在缓冲区中直到碰到换行符或缓冲区满。
 *        _IONBF：无缓冲，输出操作将直接从调用进程到目标设备，不经过缓冲区。
 * @param size 缓冲区大小，以字节为单位。size为0时，
 *        如果buf为NULL，则标准C库将为该文件流自动分配一个默认大小的缓冲区
 *        如果buf不为NULL，将size设置为0不合逻辑，行为是未定义的
 * @return int 成功返回0，失败返回非零值
 */
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

//fflush
#include <stdio.h>

/**
 * @brief 刷新一个流，如果是输出流，强制将用户空间所有缓冲的数据输出到下游设备或目标文件
 *                   如果是输入流，若是与可寻址文件关联的输入流（不包含管道和终端），丢弃缓冲区未被应用消费的数据
 *                                对管道和终端关联的输入流执行fflush()通常不会有任何效果
 *        fflush() 主要是用来处理输出流的
 * @param stream 待刷新的数据流，如果为NULL，则刷新所有打开的输出流。
 * @return int 成功返回0.失败返回EOF，并设置errno
 */
int fflush(FILE *stream);
```

- 无缓冲模式 直接输出
- 行缓冲模式 遇到换行符\n才会写入
- 全缓冲模式 缓冲区满了才会写入 可以搭配fflush来使用将缓冲区的数据推出

多线程与多进程 实现服务器接受客户端信息的区别

```c
//多线程
int main()
{   
    /*****code*****/
    while(1)
    {
        int client_fd;
        //accept
        client_fd = accept(sockfd,(struct sockaddr *)&client_addr,&client_len);
        handle_error("accept",client_fd);

        printf("与客户端 from %s at  PORT%d 文件描述符%d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);
    
        //启动一个线程 读取客户端数据并打印到对应的stdout 
        pthread_t pid_read_write;
        if(pthread_create(&pid_read_write,NULL,read_from_client_then_write,(void *)&client_fd))
        {
            perror("pthread create");
        }
        //将子线程设置为detached状态 使其终止的时候自动回收资源 不阻塞主线程
        pthread_detach(pid_read_write);
        printf("创建了子线程并设置为detached状态\n");
    }
    printf("释放资源\n");
    close(sockfd);
    return 0;
}
```

```c
    while(1)
    {
        int client_fd;
        //accept
        client_fd = accept(sockfd,(struct sockaddr *)&client_addr,&client_len);
        handle_error("accept",client_fd);

        pid_t pid = fork();
        if(pid>0)
        {            
            printf("this is father, pid is %d, continue accepting...\n", getpid());

            // 父进程不需要处理client_fd，释放文件描述符，使其引用计数减一，以便子进程释放client_fd后，其引用计数可以减为0,从而释放资源
            close(client_fd);
        }
        else if(pid ==0)
        {
            // 子进程不需要处理sockfd，释放文件描述符，使其引用计数减一
            close(sockfd);
            printf("与客户端 from %s at  PORT%d 文件描述符%d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);
            printf("新的服务端pid为: %d\n", getpid());
            // 读取客户端数据，并打印到 stdout
            read_from_client_then_write((void *)&client_fd);

            // 释放资源并终止子进程
            close(client_fd);
            exit(EXIT_SUCCESS);
        }
    }
    printf("释放资源\n");
    close(sockfd);

    return 0;
```

可以看出 多线程的处理方式是 accept一个客户端请求后就创建一个一个线程调用函数

多进程的处理方式是 accrpt一个客户端就fork一个子进程 其中父进程不从client里面读取信息  子进程调用函数来处理cliend_fd不出力sockfd 最后通过父进程来关闭sockfd
