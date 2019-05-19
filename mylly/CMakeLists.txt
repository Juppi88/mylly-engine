# 
# This is the CMake file for the Mylly engine library.
# The recommended way to run this is in a separate build directory, e.g.
# 
# mkdir build && cd build
# cmake ..
# 

cmake_minimum_required(VERSION 3.0)
project(Mylly)

# Define an option to override system compiler with clang.
option(USE_CLANG "build library with clang" ON)

if(USE_CLANG)
	set(CMAKE_C_COMPILER "clang")
endif(USE_CLANG)

# Compiler flags.
set(CMAKE_C_FLAGS         "-Wall -Wextra -DCGLM_ALL_UNALIGNED")
set(CMAKE_C_FLAGS_DEBUG   "-g -D_DEBUG -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O4 -DNDEBUG")

# List of dependencies located in non-standard directories.
find_package(OpenGL REQUIRED)
find_package(PNG REQUIRED) # libpng will automatically include zlib
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${PNG_INCLUDE_DIRS})
include_directories(${JPEG_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})

# Define where engine header files are located.
include_directories(
	.
	../external
)

# Define what source files are used.
file(GLOB_RECURSE mylly_src
    "*.h"
    "*.c"
)

# Create a static library.
add_library(mylly ${mylly_src})
