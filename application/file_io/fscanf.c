#include <stdio.h>
int main(int argc,char **argv)
{
    /*
        int fscanf (FILE *__restrict __stream,  任务句柄
		const char *__restrict __format, ...)   读取的匹配表达式
        return: 返回成功的参数个数 失败返回0  报错或结束返回EOF 
    */
    char *filename = "user.txt";
    FILE * ioFile = fopen(filename,"r");

    if(ioFile == NULL)
    {
        perror("fopen file error");
        return -1;
    }
    else
    {
        printf("fopen file success\n");
    }
    char name[50];
    int age;
    char wife[50];
    int ret;

    while(fscanf(ioFile,"%s %d %s\n",name,&age,wife) != EOF)
    {
        printf("%s在%d的时候就爱上了%s\n",name,age,wife);
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
