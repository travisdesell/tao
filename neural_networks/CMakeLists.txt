include_directories(${Boost_INCLUDE_DIR})

add_library(neural_networks time_series_neural_network edge vision_neural_network convolutional_neural_network ${OpenCV_LIBRARIES})
