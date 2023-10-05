//
// Created by andrey on 2/5/21.
//

#ifndef CORE_BLOCKINGQUEUE_H
#define CORE_BLOCKINGQUEUE_H
#include <queue>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace CoreUtils {
template <typename T>
class BlockingQueue {
public:
    BlockingQueue() : mMutex(), mCv(), mQueue() {}
    BlockingQueue(const BlockingQueue &rhs) = delete;
    BlockingQueue &operator=(const BlockingQueue &rhs) = delete;

    void put(T &&task)
    {
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mQueue.emplace(std::forward<T>(task));
        }
        mCv.notify_all();
    }

    void stop()
    {
        mStopped=true;
        mCv.notify_all();
    }

    T& pick()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait(lock, [&] { return !mQueue.empty() || mStopped; });
        if (!mStopped) {
            return mQueue.front();
        }
        return mEmpty;
    }

    void revolve()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait(lock, [&] { return !mQueue.empty() || mStopped; });
        if (!mStopped) {
            mQueue.push(std::move(mQueue.front()));
        }
    }

    void pop()
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mCv.wait(lock, [&] { return !mQueue.empty() || mStopped; });
        if (!mQueue.empty()) {
            mQueue.pop();
        }
    }

    inline bool stopped() const
    {
        return mStopped;
    }

private:
    std::mutex mMutex;
    std::condition_variable mCv;
    std::queue<T> mQueue;
    bool mStopped = false;
    T mEmpty{};
};
}
#endif  // CORE_BLOCKINGQUEUE_H
