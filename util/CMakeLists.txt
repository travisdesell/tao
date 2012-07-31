include_directories(${Boost_INCLUDE_DIR})

message(STATUS "boost include dir: ${Boost_INCLUDE_DIR}")

add_library(tao_util recombination statistics matrix)
target_link_libraries(tao_util undvc_common ${BOOST_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})

add_executable(matrix_mul_test matrix)
target_link_libraries(matrix_mul_test undvc_common ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
set_target_properties(matrix_mul_test PROPERTIES COMPILE_FLAGS -DMATRIX_MUL_TEST)

add_executable(matrix_inverse_test matrix)
target_link_libraries(matrix_inverse_test undvc_common ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
set_target_properties(matrix_inverse_test PROPERTIES COMPILE_FLAGS -DMATRIX_INVERSE_TEST)
