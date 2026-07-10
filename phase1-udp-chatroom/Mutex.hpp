#ifndef __MUTEX_HPP
#define __MUTEX_HPP

#include <iostream>
#include <pthread.h>

// 1. 封装原生的 pthread 互斥锁
class Mutex
{
public:
    Mutex()
    {
        // 初始化原生锁
        pthread_mutex_init(&_lock, nullptr);
    }

    ~Mutex()
    {
        // 销毁原生锁
        pthread_mutex_destroy(&_lock);
    }

    void Lock()
    {
        // 加锁
        pthread_mutex_lock(&_lock);
    }

    void Unlock()
    {
        // 解锁
        pthread_mutex_unlock(&_lock);
    }

    // 提供一个获取原生锁指针的接口（备用）
    pthread_mutex_t *GetMutex()
    {
        return &_lock;
    }

private:
    pthread_mutex_t _lock; // 原生 Linux 互斥锁
};

// 2. 封装守护锁（RAII机制，利用生命周期自动加锁解锁）
class LockGuard
{
public:
    // 构造函数：传进来一把锁，并在进门的第一时间把它锁上
    LockGuard(Mutex &mutex) : _mutex(mutex)
    {
        _mutex.Lock();
    }

    // 析构函数：当这个守护对象生命周期结束（比如出大括号）时，自动把锁解开
    ~LockGuard()
    {
        _mutex.Unlock();
    }

private:
    Mutex &_mutex; // 注意：这里必须是引用，保证锁的是外面传进来的那把锁
};

#endif