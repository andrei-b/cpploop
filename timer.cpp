//
// Created by andrey on 2/6/21.
//

#include "timer.h"
#include <thread>

namespace CoreUtils
{
     void Timer::stop()
     {
         _stopped = true;
     }

    Timer::Timer(MessageLoop &loop, unsigned interval, std::function<void(void)> handler) : _interval(interval), _loop(loop)
     {
         event = loop.addHandler(std::move(handler));
         thread = std::thread([&]() {
             while(!_stopped) {
                 _loop.postEvent(event);
                 sleeper.sleepFor(std::chrono::milliseconds(_interval));
             }
         });
     }

     Timer::~Timer()
     {
         _stopped = true;
         sleeper.wake();
         thread.join();
     }



}