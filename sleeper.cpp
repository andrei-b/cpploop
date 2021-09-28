//
// Created by andrey on 2/12/21.
//

#include "sleeper.h"

namespace CoreUtils
{

void Sleeper::sleepForever()
{
    std::unique_lock lock(m);
    cv.wait(lock,[&]{return _wake;});
}

void Sleeper::wake()
{
    std::unique_lock lock(m);
    _wake=true;
    cv.notify_all();
}

}