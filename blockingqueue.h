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
    BlockingQueue() : mutex(), cv(), queue() {}
    BlockingQueue(const BlockingQueue &rhs) = delete;
    BlockingQueue &operator=(const BlockingQueue &rhs) = delete;

    void put(const T &task)
    {
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(task);
        }
        cv.notify_all();
    }

    T take()
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&] { return !queue.empty(); });
        T front(std::move(queue.front()));
        queue.pop();
        return front;
    }
private:
    std::mutex mutex;
    std::condition_variable cv;
    std::queue<T> queue;
};
}
#endif  // CORE_BLOCKINGQUEUE_H
