#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        return -1;                \
    }

void *read_from_client_then_write(void *argv)
{
    int client_fd = *(int *)argv;

    char *read_buf=NULL;
    char *write_buf=NULL;
    
    read_buf = malloc(sizeof(char)*1024);
    write_buf = malloc(sizeof(char)*1024);

    memset(read_buf,0,1024);
    memset(write_buf,0,1024);
    //判断内存是否分配成功
    if(!read_buf)
    {
        printf("服务端读缓存创建失败 断开链接\n");
        shutdown(client_fd,SHUT_WR);
        close(client_fd);
        perror("malloc server read buf");
        return NULL;
    }
        if(!write_buf)
    {
        printf("服务端写缓存创建失败 断开链接\n");
        shutdown(client_fd,SHUT_WR);
        close(client_fd);
        perror("malloc server write buf");
        return NULL;
    }

    ssize_t read_cnt;
    ssize_t write_cnt;

    while( read_cnt = recv(client_fd,read_buf,1024,0))
    {
        if(read_cnt <0)
        {
            perror("server recv");
        }
        printf("reveive message from client_fd %d :%s\n",client_fd,read_buf);
        strcpy(write_buf,"received~\n");
        write_cnt = send(client_fd,write_buf,1024,0);
        if(write_cnt <0)
        {
            perror("send received");
        }
    }
    
    printf("客户端client_fd:%d 请求退出\n",client_fd);
    strcpy(write_buf,"receive your shutdown signal\n");
    write_cnt = send(client_fd,write_buf,1024,0);
    if(write_cnt <0)
    {
        perror("send shutdown");
    }

    printf("释放client_fd:%d资源\n",client_fd);
    shutdown(client_fd,SHUT_WR);
    close(client_fd);
    free(read_buf);
    free(write_buf);

    return NULL;
}

int main()
{   
    int sockfd,tempresult;
    struct sockaddr_in server_addr ,client_addr;
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    //端口6666
    server_addr.sin_port = htons(6666);
    //绑定0.0.0.0
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //创建socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    handle_error("socket",sockfd);

    //bind
    tempresult = bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("bind",tempresult);

    //listen
    tempresult=listen(sockfd,128);
    handle_error("listen",tempresult);

    socklen_t client_len = sizeof(client_addr);
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