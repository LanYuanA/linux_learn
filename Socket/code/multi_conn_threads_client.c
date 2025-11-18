#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        return -1;                \
    }

void *read_from_server(void *argv)
{
    int sockfd = *(int *)argv;
    char *read_buf = NULL;
    ssize_t cnt = 0;

    read_buf = malloc(sizeof(char) * 1024);
    if(!read_buf)
    {
        perror("malloc  client read buf");
        return NULL;
    }

    while((cnt = recv(sockfd,read_buf,1024,0))) //非零则处理
    {
        if(cnt < 0)
        {
            perror("recv");
        }
        fputs(read_buf,stdout);
    }

    printf("收到服务器的终止信号......\n");

    free(read_buf);

    return NULL;
}

//写标准输入到socket
void *write_to_server(void *argv)
{
    int sockfd = *(int *)argv;
    char *write_buf = NULL;
    ssize_t send_num;
    write_buf = malloc(sizeof(char *) * 1024);

    if(!write_buf)
    {
        perror("malloc client write buf");
        return NULL;
    }
    //将标准输入写入到write_buf中
    while(fgets(write_buf,1024,stdin) !=NULL)
    {
       send_num = send(sockfd,write_buf,1024,0);
       if(send_num < 0)
       {
            perror("send");
       }
    }

    printf("接收到命令行的终止信号 不再写入 关闭连接......\n");

    shutdown(sockfd,SHUT_WR);
    free(write_buf);

    return NULL;
}

int main()
{
    pthread_t pid_read;
    pthread_t pid_write;
    int sockfd,tempresult;
    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
   
    server_addr.sin_family = AF_INET;
    //端口6666
    server_addr.sin_port = htons(6666);
    //绑定0.0.0.0
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockfd = socket(AF_INET,SOCK_STREAM,0);
    handle_error("socket",sockfd);

    tempresult = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("connect",tempresult);

    //启动子线程用于读取客户端服务端数据 并打印到stdout
    pthread_create(&pid_read,NULL,read_from_server,(void *)&sockfd);
    pthread_create(&pid_write,NULL,write_to_server,(void *)&sockfd);

    pthread_join(pid_read,NULL);
    pthread_join(pid_read,NULL);

    printf("关闭资源\n");
    close(sockfd);
    return 0;
}