add_library(EvolutionaryAlgorithms evolutionary_algorithm particle_swarm differential_evolution recombination)
target_link_libraries(EvolutionaryAlgorithms undvc_common)

find_package(MySQL REQUIRED)

# find_path(BOINC_INCLUDE_DIR config.h PATHS /boinc/src/boinc/ /boinc/)

include_directories(
#	${BOINC_INCLUDE_DIR}
#	${BOINC_INCLUDE_DIR}/api
#	${BOINC_INCLUDE_DIR}/db
#	${BOINC_INCLUDE_DIR}/lib
#	${BOINC_INCLUDE_DIR}/sched
	${MYSQL_INCLUDE_DIR}
)

#find_library(boinc HINTS ${BOINC_INCLUDE_DIR}/lib)
#find_library(boinc_lib HINTS ${BOINC_INCLUDE_DIR}/lib)
#find_library(boinc_api HINTS ${BOINC_INCLUDE_DIR}/api)
#find_library(boinc_sched HINTS ${BOINC_INCLUDE_DIR}/sched)

add_library(DBEvolutionaryAlgorithms particle_swarm_db differential_evolution_db)
target_link_libraries(DBEvolutionaryAlgorithms undvc_common ${MYSQL_LIBRARIES})
#target_link_libraries(DBEvolutionaryAlgorithms boinc boinc_lib boinc_api boinc_sched ${MYSQL_LIBRARIES})
