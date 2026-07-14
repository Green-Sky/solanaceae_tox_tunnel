cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_executable(test_tunnel_packets
    test_tunnel_packets.c
)
target_link_libraries(test_tunnel_packets PRIVATE
    tunnel_packets
)
target_compile_features(test_tunnel_packets PRIVATE c_std_11)

add_test(
    NAME tunnel_packets_test
    COMMAND test_tunnel_packets
)
