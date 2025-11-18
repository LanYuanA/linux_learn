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
