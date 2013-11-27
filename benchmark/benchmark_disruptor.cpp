
#include "hulk/core/disruptor.h"
#include <iostream>

using namespace hulk;

const int NUM_MSGS = 10000000;
const int NUM_SAMPLES = 5;
const int RB_SIZE = 1024*16;

class my_reader : public reader_thread< int >
{
public:
    int _msgs_processed;

    my_reader( ring_buffer< int >& rb )
    : reader_thread< int >( rb ),
      _msgs_processed( 0 )
    {
    }

protected:
    virtual void process( const int& item )
    {
        ++_msgs_processed;
    }
};

int main( int argc, char** argv )
{
    ring_buffer< int > rb( RB_SIZE );

    // create reader and start it
    my_reader r1( rb );
    r1.start();

    // create writer
    writer< int > w1( r1.get_reader() );

    for( int i=0; i<NUM_SAMPLES; i++ )
    {
        stopwatch watch;
        watch.start();

        // write all messages
        for( int i=0; i<NUM_MSGS; i++ )
        {
            int& j = w1.next();
            j = i;
            w1.commit();
        }

        // give readers some time to finish processing
        while( r1._msgs_processed < NUM_MSGS ) {
            thread::yield();
        }

        float secs = watch.elapsed_s();
        std::cout << NUM_MSGS << " messages in " << secs << " seconds. rate " << NUM_MSGS / secs << std::endl;
    }

    r1.stop();
    r1.join();
}
