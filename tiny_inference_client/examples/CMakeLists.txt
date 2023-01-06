function(add_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME})
    target_sources(${EXAMPLE_NAME} PRIVATE src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE tie::tiny_inference_client spdlog::spdlog)
    target_compile_features(${EXAMPLE_NAME} PUBLIC cxx_std_17)
    install(TARGETS ${EXAMPLE_NAME} DESTINATION client/examples)
endfunction()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tiny_inference_client/examples)
find_package(spdlog REQUIRED)

add_example(client_example_server_info)
add_example(client_example_model_info)
add_example(client_example_model_infer)

find_package(OpenCV REQUIRED)
add_example(client_example_simple_inference)
target_link_libraries(client_example_simple_inference PRIVATE opencv::core opencv::highgui opencv::imgproc opencv::imgcodecs)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)
