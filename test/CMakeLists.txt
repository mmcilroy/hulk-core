
add_executable( test_shared_ptr test_shared_ptr.cpp )
add_executable( test_disruptor test_disruptor.cpp )
add_executable( test_tcp_connect test_tcp_connect.cpp )
add_executable( test_tcp_flood test_tcp_flood.cpp )

target_link_libraries( test_shared_ptr hulk_core pthread )
target_link_libraries( test_disruptor hulk_core pthread )
target_link_libraries( test_tcp_connect hulk_core pthread )
target_link_libraries( test_tcp_flood hulk_core pthread )

add_executable( echo_server echo_server.cpp )
target_link_libraries( echo_server hulk_core pthread )
