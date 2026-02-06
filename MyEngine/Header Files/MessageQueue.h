//
//  MessageQueue.h
//  MyEngine
//
//  Created by Priyanshu Kaushik on 09/12/2025.
//

#pragma once
#include "Message.h"
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class MessageQueue
{
public:
    void Push(std::unique_ptr<Message> msg)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(std::move(msg));
        mCondition.notify_one();
    }

    std::unique_ptr<Message> Pop()
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty()) return nullptr;

        auto msg = std::move(mQueue.front());
        mQueue.pop();
        return msg;
    }
    std::unique_ptr<Message> WaitAndPop()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCondition.wait(lock, [this] { return !mQueue.empty(); });

        auto msg = std::move(mQueue.front());
        mQueue.pop();
        return msg;
    }

private:
    std::queue<std::unique_ptr<Message>> mQueue;
    std::mutex mMutex;
    std::condition_variable mCondition;
};

