
add_library( hulk_core logger.cpp stopwatch.cpp tcp.cpp thread.cpp )
install( TARGETS hulk_core DESTINATION lib )
