cmake_minimum_required (VERSION 2.6)
project (TAO)
# The version number.
set (TAO_VERSION_MAJOR 1)
set (TAO_VERSION_MINOR 0)
 
set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -msse3")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/undvc_common/cmake/Modules/")

find_package(BOINC)
find_package(MySQL)
find_package(Boost REQUIRED COMPONENTS system)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(undvc_common)

add_subdirectory(undvc_common)
add_subdirectory(asynchronous_algorithms)
add_subdirectory(synchronous_algorithms)
add_subdirectory(util)
add_subdirectory(examples)
add_subdirectory(mpi)

link_directories(${Boost_LIBRARY_DIRS})

if (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    add_subdirectory(boinc)
else (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    message(STATUS "Either BOINC libraries (found = ${BOINC_SERVER_FOUND}) or MYSQL libraries (found = ${MYSQL_FOUND}) missing, not compiling boinc subdirectory.")
endif (BOINC_SERVER_FOUND AND MYSQL_FOUND)
