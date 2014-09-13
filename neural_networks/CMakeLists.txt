include_directories(${Boost_INCLUDE_DIR})

add_library(neural_networks time_series_neural_network edge)
#target_link_libraries(neural_networks ${UNDVC_COMMON_LIBRARY} tao_util)
