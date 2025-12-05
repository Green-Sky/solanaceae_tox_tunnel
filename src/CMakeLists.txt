cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

add_library(plugin_template
	./plugin_template.hpp
	./plugin_template.cpp
)
target_include_directories(plugin_template PUBLIC .)
target_compile_features(plugin_template PUBLIC cxx_std_17)
target_link_libraries(plugin_template PUBLIC
	imgui
)

