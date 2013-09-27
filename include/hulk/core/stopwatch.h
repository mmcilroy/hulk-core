
#ifndef _hulk_core_stopwatch_h_
#define _hulk_core_stopwatch_h_

#include <sys/time.h>

namespace hulk {

class stopwatch
{
public:
    void start();
    unsigned int elapsed_ms();
    float elapsed_s();

private:
    struct timeval start_time;
};

}

#endif
