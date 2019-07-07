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

# Path to external dependencies within the repository.
set(EXT_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../external)

####################################################################################################
##### Compiler options
####################################################################################################

# Define an option to override system compiler with clang.
option(USE_CLANG "build library with clang" ON)

if (USE_CLANG)
	set(CMAKE_C_COMPILER "clang")
endif ()

# Compiler flags.
set(CMAKE_C_FLAGS         "-DCGLM_ALL_UNALIGNED")
set(CMAKE_C_FLAGS_DEBUG   "-D_DEBUG -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O4 -DNDEBUG")

# Additional flags for GCC/clang.
if (UNIX)
	set(CMAKE_C_FLAGS       "${CMAKE_C_FLAGS} -Wall -Wextra")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
endif ()

if (MSVC)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Zi")
endif ()

####################################################################################################
##### Project definition
####################################################################################################

# Glob source files.
file(GLOB_RECURSE MYLLY_SRC
	"*.h"
	"*.c"
)

# On Windows we'll also add the source for JSMN to the engine library. On Linux JSMN is compiled
# using its own makefile (see below).
if (WIN32)
	set(MYLLY_SRC "${MYLLY_SRC};${EXT_LIB_PATH}/jsmn/jsmn.h;${EXT_LIB_PATH}/jsmn/jsmn.c")
endif (WIN32)

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

if (WIN32)
	# Define library paths manually on Windows since we're using specific versions of libraries
	# within the repository.
	set(FREETYPE_LIBRARY ${EXT_LIB_PATH}/freetype/lib/freetype.lib)
	set(JPEG_LIBRARY ${EXT_LIB_PATH}/jpeg/lib/jpeg.lib)
	set(OPENAL_LIBRARY ${EXT_LIB_PATH}/openal/libs/Win32/OpenAL32.lib)
	set(PNG_LIBRARY ${EXT_LIB_PATH}/libpng/lib/libpng.lib)
	set(ZLIB_LIBRARY ${EXT_LIB_PATH}/zlib/lib/zlib.lib)

	set(FREETYPE_INCLUDE_DIRS ${EXT_LIB_PATH}/freetype/include)
	set(JPEG_INCLUDE_DIR ${EXT_LIB_PATH}/jpeg/include)
	set(OPENAL_INCLUDE_DIR ${EXT_LIB_PATH}/openal/include)
	set(PNG_PNG_INCLUDE_DIR ${EXT_LIB_PATH}/libpng/include)
	set(ZLIB_INCLUDE_DIR ${EXT_LIB_PATH}/zlib/include)
endif ()

find_package(PNG REQUIRED) # libpng will automatically include zlib
find_package(OpenGL REQUIRED)
find_package(JPEG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenAL REQUIRED)

# 3rd party libraries which use their own Makefiles.
if (UNIX)
	add_custom_target(
		jsmn
		COMMAND make && cp libjsmn.a ${CMAKE_BINARY_DIR}
		WORKING_DIRECTORY ${EXT_LIB_PATH}/jsmn
	)

	add_dependencies(mylly jsmn)
endif ()

# Include paths for dependencies.
target_include_directories(mylly PRIVATE
	${EXT_LIB_PATH}
	${X11_INCLUDE_DIR}
	${X11_Xrandr_INCLUDE_PATH}
	${OPENGL_INCLUDE_DIRS}
	${PNG_INCLUDE_DIRS}
	${JPEG_INCLUDE_DIR}
	${FREETYPE_INCLUDE_DIRS}
	${OPENAL_INCLUDE_DIR}
)

# Link libraries for dependencies.
target_link_libraries(mylly LINK_INTERFACE_LIBRARIES
	${OPENGL_LIBRARIES}
	${PNG_LIBRARIES}
	${JPEG_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${OPENAL_LIBRARY}
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
		${X11_Xfixes_LIB}
		${CMAKE_BINARY_DIR}/libjsmn.a
		Threads::Threads
		m
	)

endif ()

if (WIN32)

	target_include_directories(mylly PRIVATE
		${EXT_LIB_PATH}/opengl # glext, wglext
		${EXT_LIB_PATH}/cglm/include # cglm
	)

	# 3rd party libraries which don't use CMake.
	add_custom_target(
		cglm
		COMMAND ${CMAKE_VS_MSBUILD_COMMAND} cglm.vcxproj /property:Configuration=Release
		WORKING_DIRECTORY ${EXT_LIB_PATH}/cglm/win
	)

	add_dependencies(mylly cglm)

endif ()
