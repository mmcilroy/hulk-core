
add_executable( shared_ptr shared_ptr.cpp )
add_executable( tcp tcp.cpp )
add_executable( echo_server echo_server.cpp )
add_executable( swmr_disruptor swmr_disruptor.cpp )
add_executable( ut_disruptor ut_disruptor.cpp )

target_link_libraries( shared_ptr hulk_core pthread )
target_link_libraries( tcp hulk_core pthread )
target_link_libraries( echo_server hulk_core )
target_link_libraries( swmr_disruptor hulk_core pthread )
target_link_libraries( ut_disruptor hulk_core pthread )
