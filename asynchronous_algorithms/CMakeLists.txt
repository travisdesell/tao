include_directories(${Boost_INCLUDE_DIR})

add_library(asynchronous_algorithms evolutionary_algorithm particle_swarm differential_evolution individual asynchronous_newton_method)
target_link_libraries(asynchronous_algorithms undvc_common tao_util)

if (MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})

    add_library(db_asynchronous_algorithms particle_swarm_db differential_evolution_db asynchronous_newton_method_db)
    target_link_libraries(db_asynchronous_algorithms undvc_common tao_util ${MYSQL_LIBRARIES})
else (MYSQL_FOUND)
    message(STATUS "MYSQL not found, not compiling database enabled evolutionary algorithms")
endif (MYSQL_FOUND)
