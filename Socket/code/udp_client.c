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