#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_rwlock_t rwlock;
int shared_data = 0;

void *lock_reader(void *argv)
{
    pthread_rwlock_rdlock(&rwlock);//获得读锁
    printf("this is %s, its value:%d\n",(char *)argv,shared_data);
    sleep(1);
    pthread_rwlock_unlock(&rwlock);//释放读锁
}

void *lock_writer(void *argv)
{
    pthread_rwlock_wrlock(&rwlock);
    int tmp = shared_data + 1;
    shared_data = tmp;
    pthread_rwlock_unlock(&rwlock);
    printf("this is %s, shared_data++\n",(char *)argv);
}

int main()
{
    pthread_rwlockattr_t attr;
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setkind_np(&attr,PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    
    pthread_rwlock_init(&rwlock,&attr);
    pthread_rwlockattr_destroy(&attr);
    pthread_t writer1, writer2, reader1, reader2, reader3, reader4, reader5, reader6;
    pthread_create(&writer1, NULL, lock_writer, "writer1");

    pthread_create(&reader1, NULL, lock_reader, "reader1");
    pthread_create(&reader2, NULL, lock_reader, "reader2");
    pthread_create(&reader3, NULL, lock_reader, "reader3");

    pthread_create(&writer2, NULL, lock_writer, "writer2");

    pthread_create(&reader4, NULL, lock_reader, "reader4");
    pthread_create(&reader5, NULL, lock_reader, "reader5");
    pthread_create(&reader6, NULL, lock_reader, "reader6");

    pthread_join(writer1, NULL);
    pthread_join(writer2, NULL);
    pthread_join(reader1, NULL);
    pthread_join(reader2, NULL);
    pthread_join(reader3, NULL);
    pthread_join(reader4, NULL);
    pthread_join(reader5, NULL);
    pthread_join(reader6, NULL);

    pthread_rwlock_destroy(&rwlock);
}