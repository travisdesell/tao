include_directories(${Boost_INCLUDE_DIR})

IF (OPENCL_INCLUDE_DIRS)
    message(STATUS "ADDING __OPENCL__ DEFINE")
    add_definitions( -D__OPENCL__ )

    include_directories(${OPENCL_INCLUDE_DIRS})

    add_library(neural_networks time_series_neural_network edge vision_neural_network convolutional_neural_network opencl_utils ${OpenCV_LIBRARIES} ${OPENCL_LIBRARIES})
ELSE (OPENCL_INCLUDE_DIRS)
    add_library(neural_networks time_series_neural_network edge vision_neural_network convolutional_neural_network ${OpenCV_LIBRARIES})
ENDIF(OPENCL_INCLUDE_DIRS)

