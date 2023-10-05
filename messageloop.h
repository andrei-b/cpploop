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
#include <variant>

using event_id_t = uint32_t;

namespace CoreUtils
{

    using Callback = std::function<void()>;

    class MessageLoop
    {
        class Callable
        {
        public:
            Callable();
            explicit Callable(Callback && callback);
            explicit Callable(Callback* callback);
            void operator ()();
        private:
            std::variant<Callback, Callback*> mCallback;
        };
    public:
        MessageLoop();
        MessageLoop(const MessageLoop &) = delete;
        MessageLoop(MessageLoop &&) = delete;
        MessageLoop &operator=(const MessageLoop &rhs) = delete;
        void run();
        void post(Callback && callback);
        void post(Callback * callaback);
        void postAndWait(const Callback &callaback);
        void exit();
    private:
        Sleeper & newSleeper();
        BlockingQueue<Callable> events;
        Sleeper mSleeper;
        bool mExit = false;
        Sleeper mRunnning;
        std::mutex mExitMutex;
        std::thread::id mRunningThreadId = (std::thread::id)0;
    };
}

#endif  // CORE_MESSAGELOOP_H
