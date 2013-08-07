add_executable( echo-server echo-server.cpp )

target_link_libraries( echo-server hulk-core )

install( TARGETS echo-server DESTINATION bin )
