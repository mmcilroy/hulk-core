
#include "hulk/core/shared_ptr.h"
#include <iostream>

using namespace hulk;

class my_object
{
public:
    my_object() {
        std::cout << "new my_object @ " << this << std::endl;
    }

    ~my_object() {
        std::cout << "del my_object @ " << this << std::endl;
    }

    void hello() {
        std::cout << "hello!\n";
    }
};

int main( int argc, char** argv )
{
    {
        shared_ptr< my_object > p1( new my_object );
        shared_ptr< my_object > p2( p1 );
        p2.reset();
        p1->hello();
    }

    {
        shared_ptr< my_object > p3( new my_object );
        p3->hello();
    }
}
