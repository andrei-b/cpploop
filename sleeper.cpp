//
// Created by andrey on 2/12/21.
//

#include "sleeper.h"

namespace CoreUtils
{

void Sleeper::sleepForever()
{
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock,[&]{return mWake;});
}

void Sleeper::wake()
{
    std::unique_lock<std::mutex> lock(m);
    mWake=true;
    cv.notify_all();
}

void Sleeper::reset()
{
    mWake = false;
}

}
