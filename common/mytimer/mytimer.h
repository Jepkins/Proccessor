#ifndef MYTIMER_H
#define MYTIMER_H

#include <stdio.h>
#include <time.h>
#include <chrono>
#include <ctime>
#include <cmath>

class timer_cl
{
    private:
        std::chrono::time_point<std::chrono::system_clock> mark = {};
        bool started = false;
    public:
        void start()
        {
            mark = std::chrono::system_clock::now();
            started = true;
        }
        auto now()
        {
            return std::chrono::system_clock::now();
        }
        size_t sectime()
        {
            return mictime() / 1'000'000;
        }
        size_t miltime()
        {
            return mictime() / 1'000;
        }
        size_t mictime()
        {
            if (started)
            {
                return (size_t)std::chrono::duration_cast<std::chrono::microseconds>(now() - mark).count();
            }
            else
                return 0;
        }
};

#endif // MYTIMER_H
