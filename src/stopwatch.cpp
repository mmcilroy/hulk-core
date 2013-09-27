
#include "hulk/core/stopwatch.h"
#include <ctime>

using namespace hulk;

void stopwatch::start()
{
    gettimeofday( &start_time, NULL );
}

unsigned int stopwatch::elapsed_ms()
{
    struct timeval now_tv;
    gettimeofday( &now_tv, NULL );

    suseconds_t us = ( now_tv.tv_usec - start_time.tv_usec ) / 1000;
    time_t s = ( now_tv.tv_sec - start_time.tv_sec ) * 1000;

    return s + us;
}

float stopwatch::elapsed_s()
{
    return elapsed_ms() / 1000.0f;
}
