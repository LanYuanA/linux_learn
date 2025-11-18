#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define INADDR_LOCAL 0xC0A80A96

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

int main(int argc,char const *argv[])
{
    int sockfd;
    int temp_err;
    int client_fd;

    pthread_t pid_read,pid_write;
    //创建服务端地址 客户端地址
    struct sockaddr_in server_addr,client_addr;
    //初始化 清零
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    //声明IPV4协议
    server_addr.sin_family = AF_INET;
    //连接本机127.0.0.1
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    //设置端口
    server_addr.sin_port = htons(6666);

    //声明IPV4协议
    client_addr.sin_family = AF_INET;
    //连接本机 192.168.10.150
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //设置端口 8888
    client_addr.sin_port = htons(8888);

    //创建socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    handle_error("socket",sockfd);

    //绑定地址
    temp_err = bind(sockfd,(struct sockaddr *)&client_addr,sizeof(client_addr));
    handle_error("bind",temp_err);

    //连接server
    temp_err = connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("connect",temp_err);

  

    //启动子线程用于读取客户端服务端数据 并打印到stdout
    pthread_create(&pid_read,NULL,read_from_server,(void *)&sockfd);
    pthread_create(&pid_write,NULL,write_to_server,(void *)&sockfd);

    pthread_join(pid_read,NULL);
    pthread_join(pid_write,NULL);

    printf("关闭资源\n");

    //close(client_fd);
    close(sockfd);
    return 0;
}