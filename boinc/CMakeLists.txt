find_package(MySQL REQUIRED)
find_package(BOINC REQUIRED)

find_path(BOINC_INCLUDE_DIR config.h PATHS /boinc/src/boinc/ /boinc/)

include_directories(
    ${PROJECT_SOURCE_DIR}/evolutionary_algorithms
	${BOINC_INCLUDE_DIR}
    ${BOINC_INCLUDE_DIR}/api
    ${BOINC_INCLUDE_DIR}/db
    ${BOINC_INCLUDE_DIR}/lib
    ${BOINC_INCLUDE_DIR}/sched
	${MYSQL_INCLUDE_DIR}
)

add_executable(ea_validator ${BOINC_INCLUDE_DIR}/sched/validator ${BOINC_INCLUDE_DIR}/sched/validate_util ${BOINC_INCLUDE_DIR}/lib/md5_file ea_validation_policy)
target_link_libraries(ea_validator EvolutionaryAlgorithms DBEvolutionaryAlgorithms ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES})

add_executable(ea_assimilator ${BOINC_INCLUDE_DIR}/sched/assimilator ea_assimilation_policy)
target_link_libraries(ea_assimilator ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES})
