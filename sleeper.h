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
    template<intmax_t Rep, intmax_t Period>
    bool sleepFor(const std::chrono::duration<long, std::ratio<Rep, Period>> & time) const
    {
        std::unique_lock lock(m);
        return !cv.wait_for(lock, time, [&]{return _wake;});
    }
    void sleepForever(); //Until He calls
    void wake();
private:
     mutable std::condition_variable cv;
     mutable std::mutex m;
     bool _wake = false;
};


}
#endif  // CORE_SLEEPER_H
