
add_executable( test_shared_ptr test_shared_ptr.cpp )
add_executable( test_tcp test_tcp.cpp )

target_link_libraries( test_shared_ptr hulk_core pthread )
target_link_libraries( test_tcp hulk_core pthread )
