# UDP通讯

UDP也用socket但是不存在握手这一步骤 在绑定地址之后 服务端不需要listen 客户端也不需要connect 服务端也不需要accept

UDP不用send和recv方法 用sendto 和recvfrom

```c
#include <sys/types.h>
#include <sys/socket.h>
/**
 * @brief 将接收到的消息放入缓冲区 buf 中。
 * 
 * @param sockfd 套接字文件描述符
 * @param buf 缓冲区指针
 * @param len 缓冲区大小
 * @param flags 通信标签，详见recv方法说明
 * @param src_addr 可以填NULL，如果 src_addr 不是 NULL，并且底层协议提供了消息的源地址，则该源地址将被放置在 src_addr 指向的缓冲区中。
 * @param addrlen 如果src_addr不为NULL，它应初始化为与 src_addr 关联的缓冲区的大小。返回时，addrlen 被更新为包含实际源地址的大小。如果提供的缓冲区太小，则返回的地址将被截断；在这种情况下，addrlen 将返回一个大于调用时提供的值。
 * @return ssize_t 实际收到消息的大小。如果接收失败，返回-1
 */
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

/**
 * @brief 向指定地址发送缓冲区中的数据（一般用于UDP模式）
 * 
 * @param sockfd 套接字文件描述符
 * @param buf 缓冲区指针
 * @param len 缓冲区大小
 * @param flags 通信标签，详细减send方法说明
 * @param dest_addr 目标地址。如果用于连接模式，该参数会被忽略
 * @param addrlen 目标地址长度
 * @return ssize_t 发送的消息大小。发送失败返回-1
 */
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
```

```c
//server
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        return -1;                \
    }

int main(int argc, char const *argv[])
{
    int sockfd;
    int temp;
    char *buf = malloc(1024);
    struct sockaddr_in server_addr,client_addr;
    memset(&server_addr,0,sizeof(server_addr));
    memset(&client_addr,0,sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6666);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    handle_error("socket",sockfd);

    temp = bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("bind",temp);  
    
    do
    {
        /* code */
        memset(buf,0,1024);
        //尝试接受数据
        socklen_t client_addr_len = sizeof(client_addr);
        temp = recvfrom(sockfd,buf,1024,0,(struct sockaddr *)&client_addr,&client_addr_len);
        handle_error("recvfrom",temp);  
        //如果客户端发来的不是EOF
        if(strncmp(buf,"EOF",3)!=0)
        {
            printf("received message from %s at PORT%d :%s\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),buf);
            strcpy(buf,"OK\n");
        }
        else
        {
            printf("received EOF from client,exiting...\n");
        }
        temp = sendto(sockfd,buf,4,0,(struct sockaddr *)&client_addr,client_addr_len);
        handle_error("sendto",temp);
    } while (strncmp(buf,"EOD",3)!=0);
    free(buf);
    return 0;
}

//client
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define handle_error(cmd, result) \
    if (result < 0)               \
    {                             \
        perror(cmd);              \
        return -1;                \
    }
int main(int argc, char const *argv[])
{
    int sockfd;
    int temp;
    char *buf = malloc(1024);
    struct sockaddr_in server_addr,client_addr;
    memset(&server_addr,0,sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(6666);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    handle_error("socket",sockfd);

    do
    {
        /* code */
        write(STDOUT_FILENO,"Type something you want to send: ", 34);
        int buf_len = read(STDIN_FILENO,buf,1023);
        temp = sendto(sockfd,buf,buf_len,0,(struct sockaddr *)&server_addr,sizeof(server_addr));
        handle_error("recvfrom",temp);  
        //清空缓冲区
        memset(buf,0,1024);
        //尝试接收数据
        temp = recvfrom(sockfd,buf,1024,0,NULL,NULL);
        handle_error("recvfrom",temp);
        if (strncmp(buf, "EOF", 3) != 0)
        {
            printf("received msg from %s at port %d: %s", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), buf);
        }
    } while (strncmp(buf,"EOF",3)!=0);
    free(buf);
    return 0;
}
```

## socket也可以用与主机的进程通信 无需经过网络协议栈 实现双向数据传输

```c
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "unix_domain.socket"
#define SERVER_MODE 1
#define CLIENT_MODE 2
#define BUF_LEN 1024

static struct sockaddr_un socket_addr;
static char *buf;
void handle_error(char *err_msg)
{
    perror(err_msg);
    unlink(SOCKET_PATH);
    exit(-1);
}

void server_mode(int sockfd)
{
    int client_fd ,msg_len;
    static struct sockaddr_un client_addr;
    if((bind(sockfd,(struct sockaddr *)&socket_addr,sizeof(socket_addr))<0))
    {
        handle_error("bind");
    }
    if(listen(sockfd,128)<0)
    {
        handle_error("listen");
    }
    socklen_t client_addr_len = sizeof(client_addr);
    if((client_fd = accept(sockfd,(struct sockaddr *)&client_addr,&client_addr_len))<0)
    {
        handle_error("accept");
    }
    write(STDOUT_FILENO,"Connect to client!\n",20);
    do
    {
        /* code */
        memset(buf,0,BUF_LEN);
        msg_len = recv(client_fd,buf,BUF_LEN,0);
        printf("received msg:%s",buf);
        if(strncmp(buf,"EOF",3)!=0)
        {
            strcpy(buf,"From server:OK!\n\0");
        }
        send(client_fd,buf,BUF_LEN,0)<0;
    } while (strncmp(buf,"EOF",3)!=0);
    
    if(shutdown(client_fd,SHUT_RDWR)<0)
    {
        handle_error("shutdown");
    }
    unlink(SOCKET_PATH);
}

void client_mode(int sockfd)
{
    int client_fd ,msg_len;
    static struct sockaddr_un client_addr;
    if(connect(sockfd,(struct sockaddr *)&socket_addr,sizeof(struct sockaddr_un))<0)
    {
        handle_error("connect");
    }

    write(STDOUT_FILENO,"Connected to server\n",21);

    do
    {
        /* code */
        printf("type you want to send\n");
        msg_len = read(STDIN_FILENO,buf,BUF_LEN);
        send(sockfd,buf,msg_len,0);
        msg_len = recv(sockfd,buf,BUF_LEN,0);
        write(STDOUT_FILENO,buf,msg_len);
    } while (strncmp(buf,"EOF",3)!=0);
}

int main(int argc,char *argv[])
{
    int fd =0;
    int mode = 0;
    if(argc == 1 ||strncmp(argv[1],"server",6)==0)
    {
        mode = SERVER_MODE;
    }
    else if(strncmp(argv[1],"client",6)==0)
    {
        mode = CLIENT_MODE;
    }
    else
    {
        printf("参数错误\n");
        exit(EXIT_FAILURE);
    }
    //addr初始化
    memset(&socket_addr,0,sizeof(struct sockaddr_un));
    buf = malloc(BUF_LEN);
    //给addr赋值
    socket_addr.sun_family =AF_UNIX;
    strcpy(socket_addr.sun_path,SOCKET_PATH);

    if((fd = socket(AF_UNIX,SOCK_STREAM,0))<0)
    {
        handle_error("socket");
    }
    //分服务端和客户端
    switch (mode)
    {
    case SERVER_MODE:
        /* code */
        server_mode(fd);
        break;
    
    case CLIENT_MODE:
        client_mode(fd);
        break;
    }
    if(shutdown(fd,SHUT_RDWR)<0)
    {
        handle_error("shutdown");
    }
    free(buf);
    exit(0);
}
```

设置socket模式为AF_UNIX 根据程序的启动方式来选择服务端和客户端 client从sockfd来获取server返回的信息 并输出在控制台 server从client_fd来获取信息 返回值写入到client_fd