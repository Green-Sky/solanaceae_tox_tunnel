cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_executable(test_tunnel_packets
	./test_tunnel_packets.c
)
target_link_libraries(test_tunnel_packets PRIVATE
	tunnel_packets
)
target_compile_features(test_tunnel_packets PRIVATE c_std_11)

add_test(
	NAME tunnel_packets_test
	COMMAND test_tunnel_packets
)

########################################

add_executable(test_pipe_reliability
	./test_pipe_reliability.cpp
)
target_link_libraries(test_pipe_reliability PRIVATE
	tunnel_packets
	pipe
)
target_compile_features(test_pipe_reliability PRIVATE cxx_std_17)

add_test(
	NAME pipe_reliability_test
	COMMAND test_pipe_reliability
)
