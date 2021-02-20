//
// Created by andrey on 2/6/21.
//

#include "timer.h"
#include "spdlog/spdlog.h"



namespace CoreUtils
{
     void Timer::stop()
     {
         exit = true;
     }

     Timer::Timer(MessageLoop &loop, int interval, std::function<void(void)> handler) : _interval(interval), _loop(loop)
     {
         event = loop.addHandler(std::move(handler));
         spdlog::trace("Creating timer");
         thread = std::thread([&]() {
                while(!exit) {
                 _loop.postEvent(event);
                 spdlog::trace("Event posted: {}", event);
                 sleeper.sleepFor(std::chrono::milliseconds(interval));
             }
             spdlog::trace("Exiting timer");
         });
     }

     Timer::~Timer()
     {
         exit = true;
         sleeper.wake();
         thread.join();
     }

     }