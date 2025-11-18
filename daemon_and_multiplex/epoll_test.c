#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define SEVER_PORT 6666
#define BUFFER_SIZE 1024
#define MAX_EVENTS 10

#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        exit(EXIT_FAILURE);       \
    }
char *read_buf = NULL;
char *write_buf = NULL;

void init_buf()
{
    read_buf = malloc(sizeof(char)*BUFFER_SIZE);
    // 判断内存是否分配成功
    if (!read_buf)
    {
        printf("服务端读缓存创建异常，断开连接\n");
        perror("malloc sever read_buf");
        exit(EXIT_FAILURE);
    }
    write_buf = malloc(sizeof(char)*BUFFER_SIZE);
    if (!write_buf)
    {
        printf("服务端写缓存创建异常，断开连接\n");
        free(read_buf);
        perror("malloc server write_buf");
        exit(EXIT_FAILURE);
    }
    memset(read_buf, 0, BUFFER_SIZE);
    memset(write_buf, 0, BUFFER_SIZE);
}

void clear_buf(char *buf)
{
    memset(buf,0,BUFFER_SIZE);
}

//把传入的文件描述符设置为非阻塞模式
//避免单次读/写/accept阻塞事件循环
void set_nonblocking(int sockfd)
{
    int opts=fcntl(sockfd,F_GETFL);//读取当前标志位
    if(opts<0)
    {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts |= O_NONBLOCK;
    int res = fcntl(sockfd,F_SETFL,opts);//写回标志位
    if(res<0)
    {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char const *argv[])
{
    init_buf();

    // 声明sockfd、clientfd和函数返回状态变量
    int sockfd, client_fd, temp_result;

    // 声明服务端和客户端地址
    struct sockaddr_in server_addr, client_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // 声明IPV4通信协议
    server_addr.sin_family = AF_INET;
    // 我们需要绑定0.0.0.0地址，转换成网络字节序后完成设置
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 端口随便用一个，但是不要用特权端口
    server_addr.sin_port = htons(SEVER_PORT);

    // 创建server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    handle_error("socket", sockfd);

    // 绑定地址
    temp_result = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    handle_error("bind", temp_result);

    // 进入监听模式
    temp_result = listen(sockfd, 128);
    handle_error("listen", temp_result);

    //将sockfd设为非阻塞模式
    set_nonblocking(sockfd);

    int epollfd,nfds;
    struct epoll_event ev,events[MAX_EVENTS];//10

    epollfd = epoll_create1(0);
    handle_error("epoll_createl",epollfd);
    ev.data.fd=sockfd;
    ev.events=EPOLLIN;

    temp_result = epoll_ctl(epollfd,EPOLL_CTL_ADD,sockfd,&ev);
    handle_error("epoll_ctl",temp_result);

    socklen_t cliaddr_len = sizeof(client_addr);

    while (1)
    {
        /* code */
        nfds = epoll_wait(epollfd,events,MAX_EVENTS,-1);//返回就绪的文件描述符
        handle_error("epoll_wait",nfds);

        for(int i=0;i<nfds;i++)
        {
            if(events[i].data.fd==sockfd)
            {
                client_fd = accept(sockfd,(struct sockaddr *)&client_addr,&cliaddr_len);
                handle_error("accept",client_fd);
                set_nonblocking(client_fd);
                printf("与客户端 from %s at PORT %d 文件描述符 %d 建立连接\n",inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);
                ev.data.fd = client_fd;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,client_fd,&ev);
            }
            else if(events[i].events&EPOLLIN)
            {
                int cnt =0,send_cnt = 0;
                client_fd = events[i].data.fd;
                while((cnt = recv(client_fd,read_buf,BUFFER_SIZE,0))>0)
                {
                    printf("reveive message from client_fd: %d: %s\n",client_fd, read_buf);
                    clear_buf(read_buf);
                    strcpy(write_buf,"received~\n");
                    send_cnt = send(client_fd,write_buf,strlen(write_buf),0);
                    handle_error("send",send_cnt);
                    clear_buf(write_buf);
                }
                if(cnt == -1&&errno == EAGAIN)
                {
                     printf("来自客户端client_fd: %d当前批次的数据已读取完毕,继续监听文件描述符集\n", client_fd);
                }
                else if(cnt ==0)
                {
                    printf("客户端client_fd:%d请求关闭连接...\n",client_fd);
                    strcpy(write_buf, "receive your shutdown signal\n");
                    send_cnt = send(client_fd, write_buf, strlen(write_buf), 0);
                    handle_error("send", send_cnt);
                    clear_buf(write_buf);
                    //从epoll文件描述符集中移除client_fd
                    printf("从epoll文件描述符集中移除client_fd:%d\n",client_fd);
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,client_fd,NULL);
                    printf("释放client_fd:%d资源\n",client_fd);
                    shutdown(client_fd,SHUT_WR);
                    close(client_fd);
                }
            }
        }
    }
    
    printf("释放资源\n");
    close(epollfd);
    close(sockfd);
    free(read_buf);
    free(write_buf);
    return 0;
}