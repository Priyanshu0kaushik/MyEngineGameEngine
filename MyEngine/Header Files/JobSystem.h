//
//  JobSystem.h
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

// Platform-specific includes for thread naming
#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <pthread.h>
#else
#include <pthread.h>
#include <sys/prctl.h>
#endif

/**
 * @class JobSystem
 * @brief A lightweight Thread Pool for executing background tasks.
 * * It Implements a Singleton pattern to manage a pool of worker threads equal to
 * the hardware concurrency limit. Uses a thread-safe queue to dispatch
 * tasks (jobs) to available workers, avoiding the overhead of creating/destroying
 * threads per task.
 * * It is mainly for work like Mesh Loading
 */
class JobSystem {
public:

    // Singleton Accessor
    static JobSystem& Get() {
        static JobSystem instance;
        return instance;
    }

    /**
     * @brief Initializes the worker threads and prepares the job queue.
     * * Spawns std::thread::hardware_concurrency() threads.
     * Each thread is assigned a debug name.
     */
    void Init() {
        unsigned int numThreads = std::thread::hardware_concurrency();
        
        if (numThreads < 2) numThreads = 2;

        m_Stop = false;
        m_Workers.reserve(numThreads);

        for (unsigned int i = 0; i < numThreads; ++i) {
            m_Workers.emplace_back([this, i]
            {
                // THREAD DEBUG NAMING
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
                        
                        // Sleep until a job is added or shutdown is requested
                        m_Condition.wait(lock, [this] { return m_Stop || !m_Jobs.empty(); });
                         
                        // Exit condition: Stop requested and queue is empty
                        if (m_Stop && m_Jobs.empty()) return;
                         
                        // Move job from queue to local stack to execute outside lock
                        job = std::move(m_Jobs.front());
                        m_Jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    /**
     * @brief Submits a function to be executed asynchronously by the thread pool.
     * @param job A std::function or lambda to execute.
     */
    void Execute(std::function<void()> job) {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Jobs.push(std::move(job));
        }
        // Wake up one sleeping worker to handle this new job
        m_Condition.notify_one();
    }

    /**
     * @brief Signals all threads to stop and waits for them to join.
     * Should be called during Engine shutdown to prevent orphan threads.
     */
    void Shutdown() {
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_Stop = true;
        }
        // Wake up all threads so they can check the m_Stop flag and exit
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
    
    // Mutex to protect queue
    std::mutex m_QueueMutex;
    // Condition variable to put threads to sleep when no work is available
    std::condition_variable m_Condition;
    // Atomic flag to signal shutdown
    std::atomic<bool> m_Stop{false};
};
