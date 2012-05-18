cmake_minimum_required (VERSION 2.6)
project (TAO)
# The version number.
set (TAO_VERSION_MAJOR 1)
set (TAO_VERSION_MINOR 0)
 
set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -ftree-vectorize -msse3")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# find_package(BOINC REQUIRED)
# include_directories(${BOINC_INCLUDE_DIRS})
# set(LIBS ${LIBS} ${BOINC_LIBRARIES})

include_directories(undvc_common)

add_subdirectory(undvc_common)
add_subdirectory(evolutionary_algorithms)
add_subdirectory(deterministic_algorithms)
add_subdirectory(examples)
