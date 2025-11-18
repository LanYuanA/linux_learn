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


void *read_from_client(void *argv)
{
    int client_fd = *(int *)argv;
    char *read_buf = NULL;
    ssize_t cnt = 0;

    read_buf = malloc(sizeof(char) * 1024);
    if(!read_buf)
    {
        perror("malloc server read buf");
        return NULL;
    }

    // 更稳健的接收循环
    while ((cnt = recv(client_fd, read_buf, sizeof(read_buf), 0)) > 0) {
        fwrite(read_buf, 1, cnt, stdout);
    }
    if (cnt == 0) {
        // 对端优雅关闭（FIN）
    } else {
        perror("recv"); // ECONNRESET 等
    }

    printf("客户端请求关闭连接......\n");

    free(read_buf);

    return NULL;
}

/*  将标准输入写入 到嵌套字里面去
*/

void *write_to_client(void *argv)
{
    int client_fd = *(int *)argv;
    char *write_buf = NULL;
    ssize_t send_num;
    write_buf = malloc(sizeof(char *) * 1024);

    if(!write_buf)
    {
        perror("malloc write buf");
        return NULL;
    }
    //将标准输入写入到write_buf中
    while(fgets(write_buf,1024,stdin) !=NULL)
    {
       send_num = send(client_fd,write_buf,1024,0);
       if(send_num < 0)
       {
            perror("send");
       }
    }

    printf("接收到命令行的终止信号 不再写入 开始退出......\n");

    shutdown(client_fd,SHUT_WR);
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
    //绑定0.0.0.0地址 转换成网络字节序
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //设置端口
    server_addr.sin_port = htons(6666);

    //创建socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    handle_error("socket",sockfd);

    //绑定地址
    temp_err = bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    handle_error("bind",temp_err);

    //进入监听模式
    temp_err = listen(sockfd,128);
    handle_error("listen",temp_err);

    //接受第一个client连接
    socklen_t clientaddr_len = sizeof(client_addr);
    client_fd = accept(sockfd,(struct sockaddr *)&client_addr,&clientaddr_len);
    handle_error("accept",client_fd);

    //打印连接信息
    printf("与客户端 from %s at PORT %d 文件描述符 %d 建立连接\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),client_fd);

    //启动子线程用于读取客户端数据 并打印到stdout
    pthread_create(&pid_read,NULL,read_from_client,&client_fd);
    pthread_create(&pid_write,NULL,write_to_client,&client_fd);

    pthread_join(pid_read,NULL);
    pthread_join(pid_write,NULL);

    printf("释放资源\n");

    close(client_fd);
    close(sockfd);
}