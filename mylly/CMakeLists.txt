####################################################################################################
# 
# This is the CMake file for the Mylly engine library.
# The recommended way to run this is in a separate build directory, e.g.
# 
# mkdir build && cd build
# cmake ..
# 
####################################################################################################

cmake_minimum_required(VERSION 3.0)
project(Mylly VERSION 0.0.1 LANGUAGES C)

####################################################################################################
##### Compiler options
####################################################################################################

# Define an option to override system compiler with clang.
option(USE_CLANG "build library with clang" ON)

if (USE_CLANG)
	set(CMAKE_C_COMPILER "clang")
endif (USE_CLANG)

# Compiler flags.
set(CMAKE_C_FLAGS         "-Wall -Wextra -DCGLM_ALL_UNALIGNED")
set(CMAKE_C_FLAGS_DEBUG   "-g -D_DEBUG -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O4 -DNDEBUG")

####################################################################################################
##### Project definition
####################################################################################################

file(GLOB_RECURSE MYLLY_SRC
	"*.h"
	"*.c"
)

# Create a static library from the engine source.
add_library(mylly ${MYLLY_SRC})

# Include public headers.
target_include_directories(mylly PUBLIC INTERFACE_INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
)

# This is a hack - figure out how to achieve this in a nicer way.
set(MYLLY_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/.."
	CACHE INTERNAL ""
)

####################################################################################################
##### Dependencies
####################################################################################################

# 3rd party libraries with CMake packaging.
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED) # libpng will automatically include zlib
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)

# 3rd party libraries which don't use CMake.
# TODO: Fix this for Windows!
add_custom_target(
	jsmn
	COMMAND make && cp libjsmn.a ${CMAKE_BINARY_DIR}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../external/jsmn
)

add_dependencies(mylly jsmn)

# Include paths for dependencies.
target_include_directories(mylly PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../external
	${X11_INCLUDE_DIR}
	${X11_Xrandr_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
)

# Link libraries for dependencies.
target_link_libraries(mylly LINK_INTERFACE_LIBRARIES
	${OPENGL_gl_LIBRARY}
	${PNG_LIBRARIES}
	${JPEG_LIBRARIES}
	${FREETYPE_LIBRARIES}
)

####################################################################################################
##### Platform specific dependencies and their includes
####################################################################################################

if (UNIX)

	set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
	set(THREADS_PREFER_PTHREAD_FLAG TRUE)

	find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)

    target_include_directories(mylly PRIVATE
		${X11_INCLUDE_DIR}
		${X11_Xrandr_INCLUDE_PATH}
	)

	target_link_libraries(mylly LINK_INTERFACE_LIBRARIES
		${X11_LIBRARIES}
		${X11_Xrandr_LIB}
		${CMAKE_BINARY_DIR}/libjsmn.a
		Threads::Threads
		m
	)

endif (UNIX)
