cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

########################################

add_library(plugin_plugin_template MODULE
	./plugin_plugin_template.cpp
)
target_compile_features(plugin_plugin_template PUBLIC cxx_std_17)
set_target_properties(plugin_plugin_template PROPERTIES
	C_VISIBILITY_PRESET hidden
)
target_compile_definitions(plugin_plugin_template PUBLIC ENTT_API_IMPORT)
target_link_libraries(plugin_plugin_template PUBLIC
	solanaceae_plugin
	plugin_template
)

