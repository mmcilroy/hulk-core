
// single writer multiple reader test

#include "hulk/core/disruptor.h"
#include <iostream>

using namespace hulk::core;

class my_reader : public reader_thread< int >
{
public:
    my_reader( const std::string& id, ring_buffer< int >& rb )
    : reader_thread< int >( rb ),
      _id( id )
    {
    }

    my_reader( const std::string& id, reader_thread< int >& rt )
    : reader_thread< int >( rt ),
      _id( id )
    {
    }

protected:
    virtual void process( const int& item )
    {
        std::cout << _id << " process " << item << std::endl;
    }

private:
    const std::string _id;
};

int main( int argc, char** argv )
{
    ring_buffer< int > rb( 1024 );

    my_reader m1( "r1", rb );
    my_reader m2( "r2", m1 );

    m2.start();
    m1.start();

    writer< int > w1( m2.get_reader() );

    for( int i=0; i<1000000; i++ )
    {
        int& j = w1.next();
        j = i;
        w1.commit();
    }

    m1.join();
}
