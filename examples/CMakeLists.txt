include_directories (${PROJECT_SOURCE_DIR}/evolutionary_algorithms)
include_directories (${PROJECT_SOURCE_DIR}/deterministic_algorithms)
include_directories (${Boost_INCLUDE_DIR})

add_executable(StandardBenchmarks standard_benchmarks)
target_link_libraries(StandardBenchmarks undvc_common EvolutionaryAlgorithms)

if (MYSQL_FOUND)
    include_directories (${MYSQL_INCLUDE_DIR})
    add_executable(StandardBenchmarksDB standard_benchmarks_db)
    target_link_libraries(StandardBenchmarksDB undvc_common EvolutionaryAlgorithms DBEvolutionaryAlgorithms ${MYSQL_LIBRARIES})
else (MYSQL_FOUND)
    message(STATUS "MYSQL not found, not compiling database enabled examples")
endif (MYSQL_FOUND)
