#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include "Logger.hpp"

using namespace NS_LOG_MODULE;

// 缺省的线程池大小：并不是越多越好，一般跟 CPU 核心数相关，这里先给一个常见的经验值
static const int default_thread_num = 4;

// =========================================================================
// 线程池设计说明：
// 1. 内部维护一个"任务队列"(_task_queue)，队列里存的是 std::function<void()>，
//    也就是"待执行的任务"，可以是任意无参数、无返回值的可调用对象（函数/Lambda都行）
// 2. 启动时提前创建好固定数量的 worker 线程，线程创建后就一直"卡"在循环里，
//    没任务的时候用条件变量阻塞休眠，不占用 CPU；有新任务来了才被唤醒去处理
// 3. 外部通过 Push() 往队列里丢任务，谁闲着谁就去抢这个任务
// 4. 线程池被销毁时，要能优雅地通知所有线程"别等了，退出吧"，然后逐一 join 等它们退出
// =========================================================================
class ThreadPool
{
public:
    ThreadPool(int thread_num = default_thread_num)
        : _thread_num(thread_num), _is_running(false)
    {}

    ~ThreadPool()
    {
        Stop(); // 对象销毁时，确保线程池已经停干净了，避免线程泄漏
    }

    // 禁止拷贝：线程池内部管理着一堆线程和锁，拷贝出来两份会导致资源混乱
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // 单例接口：全局只有一份线程池，避免多处各自创建导致线程资源膨胀
    static ThreadPool *Instance()
    {
        static ThreadPool instance;
        return &instance;
    }

    // 启动线程池：把 worker 线程一个个创建出来
    void Start()
    {
        _is_running = true;
        for (int i = 0; i < _thread_num; ++i)
        {
            // 每个线程跑的都是同一个函数：Worker()，内部是个死循环，不断从队列里取任务
            _threads.emplace_back(&ThreadPool::Worker, this);
        }
        LOG(INFO) << "thread pool start success, thread num: " << _thread_num;
    }

    // 停止线程池：通知所有线程退出，并等待它们真正结束
    void Stop()
    {
        if (!_is_running) return; // 已经停过了，不要重复停

        {
            // 修改 _is_running 之前要加锁，避免和 Worker() 里判断这个标志位产生竞态
            std::unique_lock<std::mutex> lock(_mtx);
            _is_running = false;
        }
        _cond.notify_all(); // 唤醒所有还在阻塞等待的线程，让它们发现 _is_running 已经是 false 了

        for (auto &t : _threads)
        {
            if (t.joinable()) t.join(); // 等每个线程真正跑完退出，避免主线程先结束导致资源被强制回收
        }
        _threads.clear();
        LOG(INFO) << "thread pool stop success";
    }

    // 往任务队列里塞一个新任务，外部（比如 UdpServer 的主线程）调这个接口
    void Push(const std::function<void()> &task)
    {
        {
            std::unique_lock<std::mutex> lock(_mtx);
            _task_queue.push(task);
        }
        _cond.notify_one(); // 只需要唤醒一个空闲线程来处理这一个新任务即可，没必要全部唤醒
    }

private:
    // 每个 worker 线程实际执行的函数：一个循环，不断从队列取任务、执行任务
    void Worker()
    {
        while (true)
        {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(_mtx);

                // 队列空、且线程池还在运行 → 就在这里阻塞休眠，不空转浪费 CPU
                // 用 while 而不是 if，是为了防止"虚假唤醒"（极少数情况下条件变量可能无端被唤醒）
                _cond.wait(lock, [this]() {
                    return !_task_queue.empty() || !_is_running;
                });

                // 被唤醒后，如果线程池要停了，并且队列也空了，说明真的没活干了，退出循环
                if (!_is_running && _task_queue.empty())
                {
                    return;
                }

                // 走到这里，说明队列里有任务，取出来（还在锁保护范围内，保证取任务这一步是安全的）
                task = _task_queue.front();
                _task_queue.pop();
            } // 锁在这里被释放，真正执行任务时不持锁，避免长时间占用锁导致其他线程等待

            task(); // 真正执行任务（比如：查字典 + 把结果发回给客户端）
        }
    }

private:
    int _thread_num;                          // 线程池里线程的数量
    std::vector<std::thread> _threads;         // 存放所有 worker 线程对象
    std::queue<std::function<void()>> _task_queue; // 任务队列
    std::mutex _mtx;                           // 保护任务队列的互斥锁
    std::condition_variable _cond;             // 配合互斥锁，用来实现"没任务就休眠，有任务就唤醒"
    std::atomic<bool> _is_running;             // 线程池是否还在运行
};
