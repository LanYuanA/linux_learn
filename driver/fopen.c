#include <stdio.h>
int main(int argc,char **argv)
{
    /*
    * FILE *fopen (const char *__restrict __filename,  字符串表示要打开的 文件名称
		    const char *__restrict __modes) 访问模式
            (1)r:只读模式
            2 w:只写模式 如果文件存在清空文件 不存在创建新文件
            3 a:追加写模式 如果文件存在则写入内容追加到文件末尾 不存在创建新文件
            4 r+:读写模式 文件必须存在
            5 w+:读写模式 如果文件存在清空文件 不存在创建新文件
            6 a+:读写模式 如果文件存在则写入内容追加到文件末
         __attribute_malloc__ __attr_dealloc_fclose __wur;

         returns:成功返回FILE * 结构体文件指针 失败返回NULL
    */
    char *filename = "io.txt";
    FILE * ioFile = fopen(filename,"w");
    if(ioFile == NULL)
    {
        perror("fopen file error");
        return -1;
    }
    else
    {
        printf("fopen file success\n");
    }
    return 0;

}
