#ifndef MYTIMER_H
#define MYTIMER_H

#include <stdio.h>
#include <time.h>

class timer_cl
{
    private:
        long int mark = 0;
        bool started = false;
    public:
        void start()
        {
            mark = clock();
            started = true;
        }
        long int time()
        {
            return clock() - mark;
        }
        void end()
        {
            started = false;
        }
};

#endif // MYTIMER_H
