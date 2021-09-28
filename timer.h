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
    Timer(MessageLoop & loop, unsigned interval, std::function<void(void)> handler);
    ~Timer();

    void stop();
private:
    unsigned _interval = 0;
    MessageLoop & _loop;
    bool _stopped = false;
    int event = 0;
    std::thread thread;
    Sleeper sleeper;
};

}

#endif  // CORE_TIMER_H
