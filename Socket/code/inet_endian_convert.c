#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char const *argv[])
{
    //网络地址赋值
    struct  sockaddr_in server_addr; //internet socket addr
    struct  in_addr server_in_addr; //Internet addr
    in_addr_t server_in_addr_t;
    memset(&server_addr,0,sizeof(server_addr));
    memset(&server_in_addr,0,sizeof(server_in_addr));
    memset(&server_in_addr_t,0,sizeof(server_in_addr_t));

    //打印16进制ip地址作为对照
    printf("192.168.6.101 的16进制表示为 0x%X 0x%X 0x%X 0x%X\n", 192, 168, 6, 101);
    printf("192.168.137.1 的16进制表示为 0x%X 0x%X 0x%X 0x%X\n", 192, 168, 137, 1);
    //不推荐使用 因为返回错误的-1也是个有效地址
    server_in_addr_t = inet_addr("192.168.6.101");
    printf("inet_addr convert:0x%X\n",server_in_addr_t);

    inet_aton("192.168.6.101", &server_in_addr);
    printf("inet_aton convert: 0x%X\n", server_in_addr.s_addr);

    //推荐将字符串转sockin_addr结构体
    inet_pton(AF_INET, "192.168.6.101", &server_addr.sin_addr);
    printf("inet_pton 后 server_addr.sin_addr 的16进制表示为 0x%X\n", server_addr.sin_addr.s_addr);

    //结构体转字符串
    printf("通过inet_ntoa打印inet_pton转化后的地址: %s\n", inet_ntoa(server_addr.sin_addr));

    //打印网络号部分
    printf("netword number section: 0x%X\n",inet_netof(server_addr.sin_addr));

    //使用本地网络地址和网络号 拼接成in_addr
    server_addr.sin_addr = inet_makeaddr(inet_netof(server_addr.sin_addr), inet_lnaof(server_addr.sin_addr));

    //以网络字节序16进制打印拼接的地址
    printf("inet_makeaddr: 0x%X\n", server_addr.sin_addr.s_addr);

    //打印拼接的地址
    printf("通过inet_ntoa打印inet_makeaddr拼接后的地址%s\n", inet_ntoa(server_addr.sin_addr));

    return 0;

}