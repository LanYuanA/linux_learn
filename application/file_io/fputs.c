#include <stdio.h>
int main(int argc,char **argv)
{
    /*
         fputs (const char *__restrict __s, 要写入的字符串
         FILE *__restrict __stream);    文件句柄
         returns:成功返回非负数 失败返回EOF
    */
    char *filename = "io.txt";
    FILE * ioFile = fopen(filename,"a+");

    if(ioFile == NULL)
    {
        perror("fopen file error");
        return -1;
    }
    else
    {
        printf("fopen file success\n");
    }
    
    int ret = fputs("hello\n",ioFile);
    if(ret == EOF)
    {
        printf("fputs error\n");
    }
    else
    {
        printf("fputs success\n");
    }

    int ret2 = fclose(ioFile);
    if(ret2 != 0)
    {
        printf("close file error\n");
        return 1;
    }
    else
    {
        printf("close file success\n");
    }
    return 0;

}
