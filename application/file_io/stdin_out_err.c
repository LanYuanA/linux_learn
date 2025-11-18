#include <stdio.h>
#include <stdlib.h>

int main (int argc ,char **argv[])
{
    char *ch = malloc(100);
    /*stdin*/
    fgets(ch,100,stdin);
    printf("您好，%s\n",ch);

    /*stdout
    标准输出FILE * 写入这个文件流会将数据输出到控制台
    */
    fputs(ch,stdout);

    /*stderr: 错误输出FILE * 一般用于输出错误日志*/
    fputs(ch,stderr);
    return 0;
}