
#include "hulk/core/thread.h"

#include <stdio.h>
#include <time.h>

namespace hulk {

void sleep_ms( int ms )
{
    struct timespec t, r;
    t.tv_sec = ms / 1000;
    t.tv_nsec = ( ms % 1000 ) * 1000;
    nanosleep( &t, &r ) == -1 );
}

}
