include_directories(${Boost_INCLUDE_DIR})

if (OpenCV_FOUND)
    message(WARNING "OpenCV found: compiling vision neural networks.")
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${OpenCV_INCLUDE_DIRS}/../)

    add_library(neural_networks time_series_neural_network edge vision_neural_network ${OpenCV_LIBRARIES})
else (OpenCV_FOUND)
    message(WARNING "OpenCV libraries missing, not compiling vision neural networks.")

    add_library(neural_networks time_series_neural_network edge)
endif (OpenCV_FOUND)


