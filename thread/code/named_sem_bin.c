#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

int main()
{
    //创建虚拟内存名称后打开
    char *sem_name = "/semshm";
    char *name = "/myshm";

    sem_t *sem = sem_open(sem_name,O_CREAT | O_RDWR,0666,1);

    int fd = shm_open(name,O_CREAT | O_RDWR,0666);
    
    ftruncate(fd,sizeof(int));

    //映射到内存上
    int *share= mmap(NULL,sizeof(int),PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);

    close(fd);

    *share = 0;

    int pid = fork();
    if(pid < 0)
    {
        perror("fork");
    }

    //都要执行的代码
    sem_wait(sem);
    int tmp = *share +1;
    sleep(1);
    *share = tmp;
    sem_post(sem);

    sem_close(sem);
    if(pid > 0)
    {
        //父进程要执行的代码

        //等待子进程结束
        waitpid(pid,NULL,0);
        printf("this is father, child finished\n");
        printf("the final value is %d\n", *share);
        sem_unlink(sem_name);
    }
    //解除共享内存的映射
    munmap(share,sizeof(int));
    
    if(pid > 0)
    {
        shm_unlink(name);
    }
    return 0 ;
}