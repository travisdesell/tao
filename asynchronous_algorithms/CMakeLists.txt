add_library(asynchronous_algorithms evolutionary_algorithm particle_swarm differential_evolution individual asynchronous_newton_method asynchronous_genetic_search ant_colony_optimization neat ant_colony_optimization_new)
#add_library(asynchronous_algorithms evolutionary_algorithm particle_swarm differential_evolution individual asynchronous_newton_method asynchronous_genetic_search)
target_link_libraries(asynchronous_algorithms tao_util)

if (MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})
    include_directories(${MYSQL_INCLUDE_DIR}/mysql)

    add_library(db_asynchronous_algorithms particle_swarm_db differential_evolution_db asynchronous_newton_method_db)
    target_link_libraries(db_asynchronous_algorithms tao_util ${MYSQL_LIBRARIES})
else (MYSQL_FOUND)
    message(STATUS "MYSQL not found, not compiling database enabled evolutionary algorithms")
endif (MYSQL_FOUND)
