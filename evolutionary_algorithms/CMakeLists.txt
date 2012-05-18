add_library(EvolutionaryAlgorithms evolutionary_algorithm particle_swarm differential_evolution recombination)
target_link_libraries(EvolutionaryAlgorithms DeterministicAlgorithms undvc_common)

set(BOINC_DIR /Users/deselt/Software/boinc/mac_build/build/Development)

find_package(MySQL)

include_directories(/opt/local/include/mysql5/mysql/)

include_directories(/Users/deselt/Software/boinc/)
include_directories(/Users/deselt/Software/boinc/api/)
include_directories(/Users/deselt/Software/boinc/lib/)
include_directories(/Users/deselt/Software/boinc/sched/)


add_library(BOINCEvolutionaryAlgorithms particle_swarm_db)
target_link_libraries(BOINCEvolutionaryAlgorithms ${BOINC_DIR}/libboinc.a ${BOINC_DIR}/libboinc_api.a MySQL)
