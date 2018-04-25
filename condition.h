//
// Created by luomingshun on 2018/4/24.
//

#ifndef MYTHREADPOOL_CONDITION_H
#define MYTHREADPOOL_CONDITION_H

#include <pthread.h>

//封装一个互斥量和条件变量作为状态
typedef struct condition
{
    pthread_mutex_t pmutex;
    pthread_cond_t pcond;
}condition_t;

class condition_mutex
{
public:
    condition_mutex();
    ~condition_mutex();
    //对状态的操作函数
    int condition_init();//初始化
    int condition_lock();//加锁
    int condition_unlock();//解锁
    int condition_wait();//等待
    int condition_timedwait(const struct timespec *abstime);//超时等待
    int condition_signal();//唤醒一个睡眠线程
    int condition_broadcast();//唤醒所以睡眠线程
    int condition_destroy();//销毁
private:
    condition_t c_mutex;
};

#endif //MYTHREADPOOL_CONDITION_H
