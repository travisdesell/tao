add_library(clustering dbscan)
target_link_libraries(clustering ${UNDVC_COMMON_LIBRARY} tao_util)
