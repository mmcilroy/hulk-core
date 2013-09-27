
#include "hulk/core/thread.h"

namespace hulk {

void sleep_ms( int ms )
{
    struct timespec t, r;
    t.tv_sec = 0; t.tv_nsec = ms * 1000000;
    nanosleep( &t , &r );
}

}
