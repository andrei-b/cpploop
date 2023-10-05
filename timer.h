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
    Timer(MessageLoop & loop, unsigned interval, Callback && handler);
    ~Timer();

    void stop();
private:
    unsigned _interval = 0;
    MessageLoop & mLoop;
    bool mStopped = false;
    std::thread thread;
    Callback mHandler;
    Sleeper mSleeper;
};

}

#endif  // CORE_TIMER_H
