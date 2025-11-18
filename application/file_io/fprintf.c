#include <stdio.h>
int main(int argc,char **argv)
{
    /*
        fprintf (FILE *__restrict __stream, 文件句柄
         const char *__restrict __fmt, ...) 变长参数 格式化输出到文件流
         returns:成功返回写入的字符数 失败返回负数 EOF
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
    
    int ret = fprintf(ioFile,"Hello, fprintf example\n");
    if(ret < 0)
    {
        printf("fprintf error\n");
    }
    else
    {
        printf("fprintf success, wrote %d characters\n", ret);
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
