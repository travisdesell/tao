cmake_minimum_required (VERSION 2.6)
project (TAO)
# The version number.
set (TAO_VERSION_MAJOR 1)
set (TAO_VERSION_MINOR 0)
 
set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -msse3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") 

#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

include_directories(${PROJECT_SOURCE_DIR})
#message("project source dir = ${PROJECT_SOURCE_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/Modules/")

set(RAPIDJSON_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/../rapidjson/include)
MESSAGE(STATUS "RAPIDJSON include directory: ${RAPIDJSON_INCLUDE_DIR}")


#find_package(OpenCV COMPONENTS features2d nonfree flann imgproc highgui core)

find_package(BOINC)
find_package(MySQL)
find_package(OpenCL)

message(STATUS "opencl libraries: ${OPENCL_LIBRARIES}")
message(STATUS "opencl include_dirs: ${OPENCL_INCLUDE_DIRS}")

add_subdirectory(neural_networks)
add_subdirectory(asynchronous_algorithms)
add_subdirectory(synchronous_algorithms)
add_subdirectory(clustering)
add_subdirectory(util)
add_subdirectory(examples)
add_subdirectory(mpi)


if (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    add_subdirectory(boinc)
else (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    if (BOINC_SERVER_FOUND)
        message(WARNING "MYSQL libraries missing, not compiling boinc subdirectory.")
    endif (BOINC_SERVER_FOUND)
    if (MYSQL_FOUND)
        message(WARNING "BOINC libraries missing, not compiling boinc subdirectory.")
    endif (MYSQL_FOUND)
endif (BOINC_SERVER_FOUND AND MYSQL_FOUND)
