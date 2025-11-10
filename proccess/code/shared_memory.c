#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int main()
{   
    char *share;
    char shmname[100] = {0};
    sprintf(shmname,"/letter%d",getpid());
    //创建共享内存的文件描述符
    int fd = shm_open(shmname, O_CREAT | O_RDWR ,0644);
    if(fd < 0)
    {
        perror("共享内存开启失败\n");
        exit(EXIT_FAILURE);
    }   
    //将该区域扩展至100字节
    ftruncate(fd,100);
    //读写方式映射该区域到内存 开启父子共享标签 偏移量从零开始
    share = mmap(NULL,100,PROT_READ | PROT_WRITE ,MAP_SHARED,fd,0);
    if(share == MAP_FAILED)
    {
        perror("共享内存对象映射到内存失败\n");
        exit(EXIT_FAILURE);
    }
    //映射区开启完毕后关闭读取连接 不是删除
    close(fd);
    //创建子进程
    pid_t pid = fork();
    if(pid < 0 )
    {
        perror("子进程创建失败");
    }
    else if(pid == 0)
    {
        //子进程要执行的代码
        strcpy(share,"这是子进程写入的\n");
        printf("子进程完成写入\n");
    }
    else
    {
        //父进程要执行的代码
        wait(NULL);
        printf("父进程%d看到子进程%d写入共享内存的内容:%s",getpid(),pid,share);
        //释放映射区
        int ret = munmap(share,100);
        if(ret < 0 )
        {
            perror("释放映射区");
            exit(EXIT_FAILURE);
        }
    }
    //释放共享内存
    shm_unlink(shmname);
    return 0;
}