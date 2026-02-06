//
//  JobSystem
//  MyEngine
//
//  Created by Priyanshu Kaushik on 06/02/2026.
//

#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <pthread.h>
#else
#include <pthread.h>
#include <sys/prctl.h>
#endif

class JobSystem {
public:

    static JobSystem& Get() {
        static JobSystem instance;
        return instance;
    }

    void Init() {
        unsigned int numThreads = std::thread::hardware_concurrency();
        if (numThreads < 2) numThreads = 2; 

        m_Stop = false;
        for (unsigned int i = 0; i < numThreads; ++i) {
            m_Workers.emplace_back([this, i]
                {

                std::string threadName = "JobWorker_" + std::to_string(i);
                    
                #ifdef _WIN32
                    std::wstring wName(threadName.begin(), threadName.end());
                    SetThreadDescription(GetCurrentThread(), wName.c_str());
                #elif defined(__APPLE__)
                    pthread_setname_np(threadName.c_str());
                #else
                    pthread_setname_np(pthread_self(), threadName.c_str());
                #endif
                while (true) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lock(m_QueueMutex);
                        m_Condition.wait(lock, [this] { return m_Stop || !m_Jobs.empty(); });
                        
                        if (m_Stop && m_Jobs.empty()) return;
                        
                        job = std::move(m_Jobs.front());
                        m_Jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    void Execute(std::function<void()> job) {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Jobs.push(std::move(job));
        }
        m_Condition.notify_one();
    }

    void Shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }
        m_Condition.notify_all();
        for (std::thread& worker : m_Workers) {
            if (worker.joinable()) worker.join();
        }
        m_Workers.clear();
    }

private:
    JobSystem() = default;
    ~JobSystem() { Shutdown(); }

    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Jobs;
    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    std::atomic<bool> m_Stop{false};
};
