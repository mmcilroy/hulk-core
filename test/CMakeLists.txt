
add_executable( echo-server echo-server.cpp )
add_executable( swmr-disruptor swmr-disruptor.cpp )
add_executable( ut-disruptor ut-disruptor.cpp )
add_executable( shared_ptr shared_ptr.cpp )

target_link_libraries( echo-server hulk-core )
target_link_libraries( swmr-disruptor hulk-core pthread )
target_link_libraries( ut-disruptor hulk-core pthread )
target_link_libraries( shared_ptr hulk-core pthread )
