set(PROJECT_NAME tiny_inference_server)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE tie::server_core)
install(TARGETS ${PROJECT_NAME} DESTINATION .)
