add_library(EvolutionaryAlgorithms evolutionary_algorithm particle_swarm differential_evolution recombination)
target_link_libraries(EvolutionaryAlgorithms undvc_common)

include_directories(${MYSQL_INCLUDE_DIR})

if (MYSQL_FOUND)
    add_library(DBEvolutionaryAlgorithms particle_swarm_db differential_evolution_db)
    target_link_libraries(DBEvolutionaryAlgorithms undvc_common ${MYSQL_LIBRARIES})
else (MYSQL_FOUND)
    message(STATUS "MYSQL not found, not compiling database enabled evolutionary algorithms")
endif (MYSQL_FOUND)
