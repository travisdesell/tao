find_package(MPI REQUIRED)
#find_package(CUDA)

if (MPI_FOUND)
    set (CMAKE_CXX_COMPILE_FLAGS ${CMAKE_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS})
    set (CMAKE_CXX_LINK_FLAGS ${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS})
    include_directories(${MPI_INCLUDE_PATH})

    #    if (CUDA_FOUND)
    #    add_definitions( -DCUDA )
    #    cuda_add_library(mpi_algorithms mpi_genetic_algorithm mpi_particle_swarm mpi_differential_evolution master_worker assign_device)
    #    target_link_libraries(mpi_algorithms asynchronous_algorithms tao_util ${MPI_LIBRARIES} ${CUDA_LIBRARIES})
    #else (CUDA_FOUND)
        add_library(mpi_algorithms mpi_genetic_algorithm mpi_particle_swarm mpi_differential_evolution master_worker mpi_ant_colony_optimization mpi_ant_colony_optimization_new)
        target_link_libraries(mpi_algorithms asynchronous_algorithms tao_util ${MPI_LIBRARIES})
    #endif (CUDA_FOUND)

endif (MPI_FOUND)
