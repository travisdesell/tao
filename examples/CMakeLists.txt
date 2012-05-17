include_directories ("${PROJECT_SOURCE_DIR}/evolutionary_algorithms")
include_directories ("${PROJECT_SOURCE_DIR}/deterministic_algorithms")

add_executable(StandardBenchmarks standard_benchmarks)
target_link_libraries(StandardBenchmarks EvolutionaryAlgorithms undvc_common)

add_executable(EconMrsyx econ_mrsyx)
target_link_libraries(EconMrsyx EvolutionaryAlgorithms undvc_common)
