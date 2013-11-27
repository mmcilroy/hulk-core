
#include "hulk/core/disruptor.h"
#include <cassert>
#include <iostream>

using namespace hulk;

const int NUM_MSGS = 1000000;
const int RB_SIZE = 1024;

class my_reader : public reader_thread< int >
{
public:
    int _msgs_processed;

    my_reader( ring_buffer< int >& rb )
    : reader_thread< int >( rb ),
      _msgs_processed( 0 )
    {
    }

    my_reader( reader_thread< int >& rt )
    : reader_thread< int >( rt ),
      _msgs_processed( 0 )
    {
    }

protected:
    virtual void process( const int& item )
    {
        if( item == _msgs_processed ) {
            ++_msgs_processed;
        }
    }
};

int main( int argc, char** argv )
{
    ring_buffer< int > rb( RB_SIZE );

    // setup reader chain
    // r1 reads from rb, r2 from r1, r3 from r2
    my_reader r1( rb );
    my_reader r2( r1 );
    my_reader r3( r2 );

    // start the reader threads
    r3.start();
    r2.start();
    r1.start();

    // writer cannot go past r3
    writer< int > w1( r3.get_reader() );

    // start writing data
    for( int i=0; i<NUM_MSGS; i++ )
    {
        int& j = w1.next();
        j = i;
        w1.commit();
    }

    // give readers some time to finish processing
    thread::sleep( 1000 );

    std::cout << "r1: " << r1._msgs_processed << std::endl;
    std::cout << "r2: " << r2._msgs_processed << std::endl;
    std::cout << "r3: " << r3._msgs_processed << std::endl;

    // all readers should have received all messages
    assert( r1._msgs_processed == NUM_MSGS );
    assert( r2._msgs_processed == NUM_MSGS );
    assert( r3._msgs_processed == NUM_MSGS );

    r1.stop();
    r2.stop();
    r3.stop();

    r1.join();
    r2.join();
    r3.join();
}
