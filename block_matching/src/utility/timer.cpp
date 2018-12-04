#include "timer.hpp"

namespace Utility
{

Timer::Timer() : totalms(0) {}

void Timer::start() {
    startedtime = std::chrono::high_resolution_clock::now();
}

void Timer::stop() {
    auto endtime = std::chrono::high_resolution_clock::now();
    auto dur = endtime - startedtime;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

    totalms += ms;
}

void Timer::reset() {
    totalms = 0;
}

long Timer::getCumulative() {
    return totalms;
}

}