//
// Created by andrey on 2/6/21.
//

#include "timer.h"
#include <thread>

namespace CoreUtils
{
     void Timer::stop()
     {
         mStopped = true;
     }

     Timer::Timer(MessageLoop &loop, unsigned interval, Callback && handler) : _interval(interval), mLoop(loop), mHandler(std::move(handler))
     {
         thread = std::thread([this]() {
             while(!mStopped) {
                 mLoop.post(&mHandler);
                 mSleeper.sleepFor(std::chrono::milliseconds(_interval));
             }
         });
     }

     Timer::~Timer()
     {
         mStopped = true;
         mSleeper.wake();
         thread.join();
     }



}
