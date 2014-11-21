cmake_minimum_required (VERSION 2.6)
project (TAO)
# The version number.
set (TAO_VERSION_MAJOR 1)
set (TAO_VERSION_MINOR 0)
 
set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -msse3")

#set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

include_directories(${PROJECT_SOURCE_DIR})
#message("project source dir = ${PROJECT_SOURCE_DIR}")

#This will look for the undvc_common library, which should be in the
#same directory as TAO. It should be compiled in the ../undvc_common/build
#directory
FIND_PATH(UNDVC_COMMON_INCLUDE_DIR file_io.hxx
    ../undvc_common         #add addiitonal paths here if undvc_common is in a different directory
    )
MESSAGE(STATUS "UNDVC_COMMON include directory: ${UNDVC_COMMON_INCLUDE_DIR}")

FIND_LIBRARY(UNDVC_COMMON_LIBRARY
    NAMES undvc_common
    PATHS ../undvc_common/build         #add additional paths here if undvc_common is in a different directory
                                        #or if it has been build in a different directory
    PATH_SUFFIXES lib
    )
MESSAGE(STATUS "UNDVC_COMMON library: ${UNDVC_COMMON_LIBRARY}")

include_directories(${UNDVC_COMMON_INCLUDE_DIR})

#UNDVC_COMMON has the FindOpenCV.cmake, FindBOINC.cmake and FindMYSQL.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${UNDVC_COMMON_INCLUDE_DIR}/cmake/Modules/")



find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenCV COMPONENTS features2d nonfree flann imgproc highgui core)

find_package(BOINC)
find_package(MySQL)


add_subdirectory(neural_networks)
add_subdirectory(asynchronous_algorithms)
add_subdirectory(synchronous_algorithms)
add_subdirectory(util)
add_subdirectory(examples)
add_subdirectory(mpi)

link_directories(${Boost_LIBRARY_DIRS})

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
