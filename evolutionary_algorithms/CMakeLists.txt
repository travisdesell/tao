add_library(EvolutionaryAlgorithms evolutionary_algorithm particle_swarm differential_evolution recombination)
target_link_libraries(EvolutionaryAlgorithms DeterministicAlgorithms undvc_common)

find_package(MySQL REQUIRED)

find_path(BOINC_INCLUDE_DIR config.h PATHS /boinc/src/boinc/)

include_directories(
	${BOINC_INCLUDE_DIR}
	${BOINC_INCLUDE_DIR}/api
	${BOINC_INCLUDE_DIR}/db
	${BOINC_INCLUDE_DIR}/lib
	${BOINC_INCLUDE_DIR}/sched
	${MYSQL_INCLUDE_DIR}
)

find_library(boinc HINTS ${BOINC_INCLUDE_DIR}/lib REQUIRED)
find_library(boinc_lib HINTS ${BOINC_INCLUDE_DIR}/lib REQUIRED)
find_library(boinc_api HINTS ${BOINC_INCLUDE_DIR}/api REQUIRED)
find_library(boinc_sched HINTS ${BOINC_INCLUDE_DIR}/sched REQUIRED)

add_library(BOINCEvolutionaryAlgorithms particle_swarm_db)
target_link_libraries(BOINCEvolutionaryAlgorithms boinc boinc_lib boinc_api boinc_sched ${MYSQL_LIBRARIES})
