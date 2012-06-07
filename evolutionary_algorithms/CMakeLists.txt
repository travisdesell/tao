include_directories(${Boost_INCLUDE_DIR})

message(STATUS "boost include dir: ${Boost_INCLUDE_DIR}")

add_library(EvolutionaryAlgorithms evolutionary_algorithm particle_swarm differential_evolution recombination statistics)
target_link_libraries(EvolutionaryAlgorithms undvc_common ${BOOST_LIBRARIES})

if (MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})

    add_library(DBEvolutionaryAlgorithms particle_swarm_db differential_evolution_db)
    target_link_libraries(DBEvolutionaryAlgorithms undvc_common ${MYSQL_LIBRARIES} ${Boost_LIBRARIES})
else (MYSQL_FOUND)
    message(STATUS "MYSQL not found, not compiling database enabled evolutionary algorithms")
endif (MYSQL_FOUND)
