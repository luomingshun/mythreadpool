#include "threadpool.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void* mytask1(void *arg)
{
    printf("thread %d is working on task %d\n", pthread_self(), *(int*)arg);
    sleep(1);
    free(arg);
    return NULL;
}

void* mytask2(void *arg)
{
    printf("thread %d is working on task %d\n", pthread_self(), *(int*)arg);
    sleep(10);
    free(arg);
    return NULL;
}

//测试代码
int main(void)
{
    //初始化线程池，最多三个线程
    threadpool pool(3);

    int i;
    //创建十个任务
    for(i=0; i < 10; i++)
    {
        if(i == 5)
        {
            sleep(5);//故意停顿5秒钟，测试线程超时
        }

        int *arg = (int *)malloc(sizeof(int));
        *arg = i;
        if (i%2 == 0)
        {
            pool.threadpool_add_task(mytask1, arg);
        }
        else
        {
            pool.threadpool_add_task(mytask2, arg);
        }
    }
    return 0;
}