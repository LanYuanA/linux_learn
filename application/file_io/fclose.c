#include <stdio.h>
int main(int argc,char **argv)
{
    /*
        fclose (FILE *__stream)
        returns:成功返回0 失败返回EOF
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
    int result1 = fputc('A', ioFile);

    if(result1 == EOF)
    {
        printf("fputc error before fclose\n");
    }
    else
    {
        printf("fputc success before fclose\n");
    }


    int ret = fclose(ioFile);
    if(ret != 0)
    {
        printf("close file error\n");
        return 1;
    }
    else
    {
        printf("close file success\n");
    }

    int result2 = fputc('A', ioFile); // fclose 之后不能再操作文件流 否则报错
    if(result2 != 0)
    {
        printf("fputc error after fclose\n");
    }
    else
    {
        printf("fputc success after fclose\n");
    }
    return 0;

}
