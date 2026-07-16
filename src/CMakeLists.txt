cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_library(tunnel_packets
	./tunnel_packets.h
	./tunnel_packets.c
)
target_include_directories(tunnel_packets PUBLIC .)
target_compile_features(tunnel_packets PUBLIC c_std_11)

########################################

add_library(pipe
	./reliability.hpp
	./reliability.cpp
)
target_include_directories(pipe PUBLIC .)
target_compile_features(pipe PUBLIC cxx_std_17)
target_link_libraries(pipe PUBLIC
	solanaceae_util
)

########################################

add_library(plugin_template
	#./socket_udp.hpp

	./tunnel_registry.hpp

	./plugin_template.hpp
	./plugin_template.cpp
)
target_include_directories(plugin_template PUBLIC .)
target_compile_features(plugin_template PUBLIC cxx_std_17)
target_link_libraries(plugin_template PUBLIC
	solanaceae_util
	solanaceae_contact
	imgui
)

