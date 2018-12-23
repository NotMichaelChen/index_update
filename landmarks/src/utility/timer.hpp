#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

namespace Utility
{

class Timer {
public:
    Timer();

    void start();
    void stop();
    void reset();

    long getCumulative();

private:
    long totalms;
    std::chrono::time_point<std::chrono::high_resolution_clock> startedtime;
};

}

#endif