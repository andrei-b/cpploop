//
// Created by andrey on 2/12/21.
//

#ifndef CORE_SLEEPER_H
#define CORE_SLEEPER_H
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace CoreUtils {

class Sleeper
{
public:
    Sleeper()
    {}
    Sleeper(const Sleeper & rhs) = delete;
    Sleeper(Sleeper && rhs) = delete;
    Sleeper &operator=(const Sleeper &rhs) = delete;
    bool sleepFor(unsigned long time) const
    {
        std::unique_lock<std::mutex>lck(m);
        return !cv.wait_for(lck, std::chrono::milliseconds(time), [&]{return mWake;});
    }
    void sleepForever(); //Until He calls
    void wake();
    void reset();
private:
     mutable std::condition_variable cv;
     mutable std::mutex m;
     bool mWake = false;
};


}
#endif  // CORE_SLEEPER_H
