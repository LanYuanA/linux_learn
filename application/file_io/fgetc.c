#include <stdio.h>
int main(int argc,char **argv)
{
    /*
        fgetc (FILE *__stream);
        returns:成功返回读取的字符数 到文件结尾或失败返回负数 EOF
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

    int ret = fgetc(ioFile);
    if(ret == EOF)
    {
        printf("fgetc error\n");
    }
    while(ret != EOF)
    {
        printf("%c", (char)ret);
        ret = fgetc(ioFile);
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
