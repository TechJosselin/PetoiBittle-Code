file(GLOB_RECURSE app_sources
  "${CMAKE_CURRENT_LIST_DIR}/*.c"
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*.S"
)

idf_component_register(
  SRCS ${app_sources}
  INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/main"
    "${CMAKE_CURRENT_LIST_DIR}/led"
    "${CMAKE_CURRENT_LIST_DIR}/webserver"
    "${CMAKE_CURRENT_LIST_DIR}/pca9685"
    "${CMAKE_CURRENT_LIST_DIR}/mpu9250"
    "${CMAKE_SOURCE_DIR}/include"
)
