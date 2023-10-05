//
// Created by andrey on 2/5/21.
//

#ifndef CORE_MESSAGELOOP_H
#define CORE_MESSAGELOOP_H
#include "blockingqueue.h"
#include "sleeper.h"
#include <functional>
#include <list>
#include <mutex>
#include <map>
#include <condition_variable>
#include <thread>

namespace CoreUtils
{

    using Callback = std::function<void()>;

    class MessageLoop
    {
        class Callable
        {
        public:
            Callable();
            explicit Callable(Callback && callback, bool repeat);
            explicit Callable(Callback* callback, bool repeat);
            void operator ()();
            bool repeat() const;
            void setRepeat(bool repeat);
        private:
            Callback mCallback;
            Callback * mpCallback = nullptr;
            bool mRepeat = false;
        };
    public:
        MessageLoop();
        MessageLoop(const MessageLoop &) = delete;
        MessageLoop(MessageLoop &&) = delete;
        MessageLoop &operator=(const MessageLoop &rhs) = delete;
        void run();
        void post(Callback && callback, bool repeat = false);
        void post(Callback * callaback, bool repeat = false);
        void postAndWait(const Callback &callback);
        void exit();
    private:
        const std::thread::id ZeroThreadId;
        Sleeper & newSleeper();
        BlockingQueue<Callable> events;
        Sleeper mSleeper;
        bool mExit = false;
        Sleeper mRunnning;
        std::mutex mConcurrentCallMutex;
        std::thread::id mRunningThreadId = ZeroThreadId;
    };
}

#endif  // CORE_MESSAGELOOP_H
