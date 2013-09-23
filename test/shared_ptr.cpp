
#include "hulk/core/shared_ptr.h"
#include <iostream>

using namespace hulk::core;

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
    shared_ptr< my_object > p1( new my_object );
    shared_ptr< my_object > p2( p1 );

    std::cout << "pointee: " << p1.get() << std::endl;
    std::cout << "pointee: " << p2.get() << std::endl;

    p1->hello();
    p2->hello();
    (*p1).hello();
    (*p2).hello();
}