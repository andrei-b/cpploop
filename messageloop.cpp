//
// Created by andrey on 2/5/21.
//

#include "messageloop.h"
#include "sleeper.h"
#include <utility>
#include <string>

namespace CoreUtils {

MessageLoop::MessageLoop()
{
}

void CoreUtils::MessageLoop::post(Callback &&callback)
{
    events.put(Callable(std::forward<Callback>(callback)));
}

void MessageLoop::post(Callback *callaback)
{
    events.put(Callable(callaback));
}

void MessageLoop::postAndWait(const Callback &callaback)
{
    if (std::this_thread::get_id() == mRunningThreadId) {
        throw std::string("You cannot call postAndWait from the processing thread!");
    }
    std::unique_lock lck(mExitMutex);
    if (!mExit) {
        auto & sleeper = newSleeper();
        post([&]{
            callaback();
            sleeper.wake(); });
        lck.unlock();
        sleeper.sleepForever();
    }
}

void MessageLoop::run()
{
    mRunningThreadId = std::this_thread::get_id();
    while (!mExit) {
        events.pick()();
        events.pop();
    }
    mRunnning.wake();
}

void MessageLoop::exit()
{
    std::unique_lock lck(mExitMutex);
    mExit = true;
    lck.unlock();
    mSleeper.wake();
    mRunnning.sleepForever();
}

Sleeper &MessageLoop::newSleeper()
{
    mSleeper.reset();
    return mSleeper;
}

MessageLoop::Callable::Callable() : mCallback([]()->void{})
{
}

MessageLoop::Callable::Callable(Callback &&callback) : mCallback(std::move(callback))
{
}

MessageLoop::Callable::Callable(Callback *callback) : mCallback(callback)
{
}

void MessageLoop::Callable::operator()()
{
    if (std::holds_alternative<Callback>(mCallback)) {
        std::get<Callback>(mCallback)();
    } else {
        auto * fptr = std::get<Callback*>(mCallback);
        if (fptr)
            (*fptr)();
    }
}

}
