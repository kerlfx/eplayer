// #pragma once

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
private:
    std::vector<std::thread> threads;

    std::queue<std::function<void()>> funcs;
    std::mutex func_queue;

    std::condition_variable condition;

    bool stop = false;

public:
    ThreadPool(size_t threadn);
    ~ThreadPool();
    template <class F, class... Args> auto toQueue(F &&f, Args &&...args);
    size_t getQueueSize();
};

template <class F, class... Args>
auto ThreadPool::toQueue(F &&f, Args &&...args)
{
    using result_type = typename std::result_of<F(Args...)>::type;
    // using result_type = typename std::invoke_result<F, Args...>::type;
    auto func = std::make_shared<std::packaged_task<result_type()>>(
        std::bind(f, args...));

    auto res = func->get_future();

    {
        std::unique_lock<std::mutex> lk(func_queue);
        if (stop)
        {
            throw std::runtime_error("ThreadPool is stopped!");
        }
        funcs.emplace([func] { (*func)(); });
    }
    condition.notify_one();
    return res;
}

#endif // THREADPOOL_H_