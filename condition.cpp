//
// Created by luomingshun on 2018/4/24.
//

#include "condition.h"

condition_mutex::condition_mutex()
{
    condition_init();
}

condition_mutex::~condition_mutex()
{
    condition_destroy();
}

//初始化
int condition_mutex::condition_init()
{
    int status;
    if((status = pthread_mutex_init(&c_mutex.pmutex, NULL)))
        return status;

    if((status = pthread_cond_init(&c_mutex.pcond, NULL)))
        return status;

    return 0;
}

//加锁
int condition_mutex::condition_lock()
{
    return pthread_mutex_lock(&c_mutex.pmutex);
}

//解锁
int condition_mutex::condition_unlock()
{
    return pthread_mutex_unlock(&c_mutex.pmutex);
}

//等待
int condition_mutex::condition_wait()
{
    return pthread_cond_wait(&c_mutex.pcond,&c_mutex.pmutex);
}

//固定时间等待
int condition_mutex::condition_timedwait(const struct timespec *abstime)
{
    return pthread_cond_timedwait(&c_mutex.pcond,&c_mutex.pmutex, abstime);
}

//唤醒一个睡眠线程
int condition_mutex::condition_signal()
{
    return pthread_cond_signal(&c_mutex.pcond);
}

//唤醒所有睡眠线程
int condition_mutex::condition_broadcast()
{
    return pthread_cond_broadcast(&c_mutex.pcond);
}

//释放
int condition_mutex::condition_destroy()
{
    int status;
    if((status = pthread_mutex_destroy(&c_mutex.pmutex)))
        return status;

    if((status = pthread_cond_destroy(&c_mutex.pcond)))
        return status;

    return 0;
}