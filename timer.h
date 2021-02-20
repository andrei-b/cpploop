//
// Created by andrey on 2/6/21.
//

#ifndef CORE_TIMER_H
#define CORE_TIMER_H
#include "messageloop.h"
#include "sleeper.h"

namespace CoreUtils {

class Timer {
public:
    Timer(MessageLoop & loop, int interval, std::function<void(void)> handler);
    ~Timer();
    void stop();
private:
    int _interval;
    MessageLoop & _loop;
    bool exit = false;
    int event;
    std::thread thread;
    Sleeper sleeper;
};

}

#endif  // CORE_TIMER_H
