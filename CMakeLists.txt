cmake_minimum_required( VERSION 2.8 )
project( hulk_core )

include_directories( include )
add_subdirectory( src )
add_subdirectory( test )

install( FILES include/hulk/core/disruptor.h DESTINATION include/hulk/core )
install( FILES include/hulk/core/logger.h DESTINATION include/hulk/core )
install( FILES include/hulk/core/shared_ptr.h DESTINATION include/hulk/core )
install( FILES include/hulk/core/stopwatch.h DESTINATION include/hulk/core )
install( FILES include/hulk/core/tcp.h DESTINATION include/hulk/core )
install( FILES include/hulk/core/thread.h DESTINATION include/hulk/core )

enable_testing()
add_test( test_tcp test/test_tcp )
add_test( test_shared_ptr test/test_shared_ptr )
