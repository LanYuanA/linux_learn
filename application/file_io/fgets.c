#include <stdio.h>
int main(int argc,char **argv)
{
    /*
        fgets (char *__restrict __s, 接受读取的数据字符串
        int __n ,   能够接受数据的最大长度
        FILE *__restrict __stream) 文件句柄
        returns:成功返回字符串 失败返回NULL
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

    char buffer[100];
    while(fgets(buffer,sizeof(buffer),ioFile))
    {
        printf("%s", buffer);
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
