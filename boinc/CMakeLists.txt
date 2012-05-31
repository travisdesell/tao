find_package(MySQL REQUIRED)
find_package(BOINC REQUIRED)

find_path(BOINC_INCLUDE_DIR config.h PATHS /boinc/src/boinc/ /boinc/)

include_directories(
    ${PROJECT_SOURCE_DIR}
	${BOINC_INCLUDE_DIR}
    ${BOINC_INCLUDE_DIR}/api
    ${BOINC_INCLUDE_DIR}/db
    ${BOINC_INCLUDE_DIR}/lib
    ${BOINC_INCLUDE_DIR}/sched
	${MYSQL_INCLUDE_DIR}
)

add_executable(tao_validator ${BOINC_INCLUDE_DIR}/sched/validator ${BOINC_INCLUDE_DIR}/sched/validate_util ${BOINC_INCLUDE_DIR}/lib/md5_file tao_validation_policy)
target_link_libraries(tao_validator DBEvolutionaryAlgorithms EvolutionaryAlgorithms ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES})

add_executable(tao_assimilator ${BOINC_INCLUDE_DIR}/sched/assimilator tao_assimilation_policy)
target_link_libraries(tao_assimilator ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES})

#find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${BOINC_INCLUDE_DIR}/tools/
)

add_executable(tao_work_generator tao_work_generator workunit_information)
#target_link_libraries(tao_work_generator EvolutionaryAlgorithms DBEvolutionaryAlgorithms undvc_common ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
target_link_libraries(tao_work_generator DBEvolutionaryAlgorithms EvolutionaryAlgorithms undvc_common ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES})

add_library(workunit_information workunit_information)
target_link_libraries(workunit_information undvc_common ${BOINC_LIBRARIES} ${MYSQL_LIBRARIES} ${OPENSSL_LIBRARIES})
