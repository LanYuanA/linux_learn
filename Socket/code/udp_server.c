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