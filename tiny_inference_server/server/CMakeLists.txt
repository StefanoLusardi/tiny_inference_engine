set(TARGET_NAME tiny_inference_server)
add_executable(${TARGET_NAME} main.cpp)
target_link_libraries(${TARGET_NAME} PRIVATE tie::server_core)
install(TARGETS ${TARGET_NAME} DESTINATION server)
