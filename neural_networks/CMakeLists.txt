include_directories(${RAPIDJSON_INCLUDE_DIR})

IF (OPENCL_INCLUDE_DIRS)
    message(STATUS "ADDING __OPENCL__ DEFINE")
    add_definitions( -D__OPENCL__ )

    include_directories(${OPENCL_INCLUDE_DIRS})

    add_library(neural_networks neural_network time_series_neural_network edge convolutional_neural_network opencl_utils edge_new ${OpenCV_LIBRARIES} ${OPENCL_LIBRARIES})
ELSE (OPENCL_INCLUDE_DIRS)

    add_library(neural_networks neural_network time_series_neural_network edge convolutional_neural_network edge_new ${OpenCV_LIBRARIES})

ENDIF(OPENCL_INCLUDE_DIRS)

