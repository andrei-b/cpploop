//
// Created by andrey on 2/5/21.
//

#include "messageloop.h"
#include "sleeper.h"
#include <utility>
#include <string>

namespace CoreUtils {

MessageLoop::MessageLoop() : ZeroThreadId(std::thread::id(0))
{
}

void CoreUtils::MessageLoop::post(Callback &&callback, bool repeat)
{
    events.put(Callable(std::forward<Callback>(callback), repeat));
}

void MessageLoop::post(Callback *callaback, bool repeat)
{
    events.put(Callable(callaback, repeat));
}

void MessageLoop::postAndWait(const Callback &callback)
{
    std::unique_lock<std::mutex> lck(mConcurrentCallMutex);
    if (std::this_thread::get_id() == mRunningThreadId) {
        callback();
        return;
    }
    if (!mExit) {
        auto & sleeper = newSleeper();
        post([&]{
            callback();
            sleeper.wake(); });
        lck.unlock();
        sleeper.sleepForever();
    }
}

void MessageLoop::run()
{
    std::unique_lock<std::mutex> lck(mConcurrentCallMutex);
    if (mRunningThreadId != ZeroThreadId) {
        return;
    }
    mRunningThreadId = std::this_thread::get_id();
    lck.unlock();
    while (!mExit) {
        events.pick()();
        if (events.pick().repeat()) {
            events.revolve();
        }
        events.pop();
    }
    mRunnning.wake();
}

void MessageLoop::exit()
{
    std::unique_lock<std::mutex> lck(mConcurrentCallMutex);
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

MessageLoop::Callable::Callable(Callback &&callback, bool repeat) : mCallback(std::move(callback)), mRepeat(repeat)
{
}

MessageLoop::Callable::Callable(Callback *callback, bool repeat) : mpCallback(callback), mRepeat(repeat)
{
}

void MessageLoop::Callable::operator()()
{
    if (mpCallback) {
        (*mpCallback)();
    }
    else {
        mCallback();
    }
}

bool MessageLoop::Callable::repeat() const
{
    return mRepeat;
}

void MessageLoop::Callable::setRepeat(bool repeat)
{
    mRepeat = repeat;
}

}
