//
// Created by luomingshun on 2018/4/24.
//

#include "threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

//创建的线程执行
static void *thread_routine(void *arg)
{
    struct timespec abstime;
    int timeout;//超时标志位
    printf("thread %d is starting\n",pthread_self());
    threadpool_t *pool = (threadpool_t *)arg;
    while(1)
    {
        timeout = 0;
        //访问线程池之前需要加锁
        pool->ready.condition_lock();
        //空闲
        pool->idle++;
        //等待队列有任务到来 或者 收到线程池销毁通知
        while(pool->first == NULL && !pool->quit)
        {
            //否则线程阻塞等待
            printf("thread %d is waiting\n",pthread_self());
            //获取从当前时间，并加上等待时间， 设置线程的超时睡眠时间
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += 2;
            int status;
            status = pool->ready.condition_timedwait(&abstime);//该函数会解锁，允许其他线程访问，当被唤醒时，加锁
            if(status == ETIMEDOUT)
            {
                printf("thread %d wait timed out\n",pthread_self());
                timeout = 1;
                break;
            }
        }

        pool->idle--;
        if(pool->first != NULL)
        {
            //取出等待队列最前的任务，移除任务，并执行任务
            task_t *t = pool->first;
            pool->first = t->next;
            //由于任务执行需要消耗时间，先解锁让其他线程访问线程池
            pool->ready.condition_unlock();
            //执行任务
            t->run(t->arg);
            //执行完任务释放内存
            free(t);
            //重新加锁
            pool->ready.condition_lock();
        }

        //退出线程池
        if(pool->quit && pool->first == NULL)
        {
            pool->counter--;//当前工作的线程数-1
            //若线程池中没有线程，通知等待线程（主线程）全部任务已经完成
            if(pool->counter == 0)
            {
                pool->ready.condition_signal();
            }
            pool->ready.condition_unlock();
            break;
        }
        //超时，跳出销毁线程
        if(timeout == 1)
        {
            pool->counter--;//当前工作的线程数-1
            pool->ready.condition_unlock();
            break;
        }

        pool->ready.condition_unlock();
    }

    printf("thread %d is exiting\n", pthread_self());
    return NULL;

}

threadpool::threadpool(int threads)
{
    threadpool_init(threads);
}

threadpool::~threadpool()
{
    threadpool_destroy();
}
//线程池初始化
void threadpool::threadpool_init(int threads)
{
    workers.ready.condition_init();
    workers.first = NULL;
    workers.last =NULL;
    workers.counter =0;
    workers.idle =0;
    workers.max_threads = threads;
    workers.quit =0;
}


//增加一个任务到线程池
void threadpool::threadpool_add_task(void *(*run)(void *arg), void *arg)
{
    //产生一个新的任务
    task_t *newtask = (task_t *)malloc(sizeof(task_t));
    newtask->run = run;
    newtask->arg = arg;
    newtask->next=NULL;//新加的任务放在队列尾端

    //线程池的状态被多个线程共享，操作前需要加锁
    workers.ready.condition_lock();

    if(workers.first == NULL)//第一个任务加入
    {
        workers.first = newtask;
    }
    else
    {
        workers.last->next = newtask;
    }
    workers.last = newtask;  //队列尾指向新加入的线程

    //线程池中有线程空闲，唤醒
    if(workers.idle > 0)
    {
        workers.ready.condition_signal();
    }
        //当前线程池中线程个数没有达到设定的最大值，创建一个新的线程
    else if(workers.counter < workers.max_threads)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, thread_routine, &workers);
        workers.counter++;
    }
    //结束，访问
    workers.ready.condition_unlock();
}

//线程池销毁
void threadpool::threadpool_destroy()
{
    //如果已经调用销毁，直接返回
    if(workers.quit)
    {
        return;
    }
    //加锁
    workers.ready.condition_lock();
    //设置销毁标记为1
    workers.quit = 1;
    //线程池中线程个数大于0
    if(workers.counter > 0)
    {
        //对于等待的线程，发送信号唤醒
        if(workers.idle > 0)
        {
            workers.ready.condition_broadcast();
        }
        //正在执行任务的线程，等待他们结束任务
        while(workers.counter)
        {
            workers.ready.condition_wait();
        }
    }
    workers.ready.condition_unlock();
    workers.ready.condition_destroy();
}

