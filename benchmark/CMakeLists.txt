
add_executable( benchmark_disruptor benchmark_disruptor.cpp )
add_executable( benchmark_tcp benchmark_tcp.cpp )

target_link_libraries( benchmark_disruptor hulk_core pthread )
target_link_libraries( benchmark_tcp hulk_core pthread )
