
// disruptor unit test

#include "hulk/core/disruptor.h"
#include <iostream>

using namespace hulk;

void test1()
{
    ring_buffer< int > rb( 1024 );

    reader< int > r( rb );
    writer< int > w( r );

    int n = rb.size();

    for( int i=0; i<n; i++ )
    {
        int& e = w.next();
        e = i;
        w.commit();
    }

    for( int j=0; j<n; j++ )
    {
        std::cout << r.next() << std::endl;
    }
}

int main( int argc, char** argv )
{
    test1();
}