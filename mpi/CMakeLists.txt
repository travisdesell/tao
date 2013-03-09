find_package(MPI REQUIRED)

if (MPI_FOUND)
    set (CMAKE_CXX_COMPILE_FLAGS ${CMAKE_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set (CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

    include_directories(${Boost_INCLUDE_DIR})

    add_library(mpi_algorithms mpi_genetic_algorithm mpi_particle_swarm master_worker)
    target_link_libraries(mpi_algorithms asynchronous_algorithms undvc_common tao_util ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${MPI_LIBRARIES})
endif (MPI_FOUND)
