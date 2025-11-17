#pragma once

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadPool {
public:
    ThreadPool(size_t count = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < count; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { 
                            return !jobs.empty() || stop; 
                        });
                        if (stop)
                            return;
                        job = std::move(jobs.front());
                        jobs.pop();
                    }
                    job(); // execute
                }
            });
        }
    }

    void join() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& t : workers) t.join();
        workers.clear();
    }
    ~ThreadPool() {
        join();
    }

    void enqueue(std::function<void()> job) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            jobs.push(std::move(job));
        }
        cv.notify_one();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
};