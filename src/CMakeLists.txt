add_library( hulk-core stopwatch.cpp tcp.cpp logger.cpp )

install( TARGETS hulk-core DESTINATION lib )
