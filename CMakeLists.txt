cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

# TEMPLATE: search and rename every mention of:
# - solanaceae_plugin_template
# - plugin_template
# - PLUGIN_TEMPLATE
# - PluginTemplate

# cmake setup begin
project(solanaceae_plugin_template)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE ON)
else()
	set(SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE OFF)
endif()
message("II SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE " ${SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE})

option(SOLANACEAE_PLUGIN_TEMPLATE_BUILD_PLUGINS "Build the plugin_template plugins" ${SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE})

if (SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)

	# defaulting to debug mode, if not specified
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE "Debug")
	endif()

	# setup my vim ycm :D
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

	# more paths
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# external libs
add_subdirectory(./external EXCLUDE_FROM_ALL) # before increasing warn levels, sad :(

if (SOLANACEAE_PLUGIN_TEMPLATE_STANDALONE)
	set(CMAKE_CXX_EXTENSIONS OFF)

	# bump up warning levels appropriately for clang, gcc & msvc
	if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
		add_compile_options(
			-Wall -Wextra # Reasonable and standard
			-Wpedantic # Warn if non-standard C++ is used
			-Wunused # Warn on anything being unused
			#-Wconversion # Warn on type conversions that may lose data
			#-Wsign-conversion # Warn on sign conversions
			-Wshadow # Warn if a variable declaration shadows one from a parent context
		)

		if (NOT WIN32)
			#link_libraries(-fsanitize=address,undefined)
			#link_libraries(-fsanitize=undefined)
		endif()
	elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
		if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
			string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
		else()
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
		endif()
	endif()

endif()

# cmake setup end

add_subdirectory(./src)

if (SOLANACEAE_PLUGIN_TEMPLATE_BUILD_PLUGINS)
	add_subdirectory(./plugins)
endif()

