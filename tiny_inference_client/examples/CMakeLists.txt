function(add_example EXAMPLE_NAME)
    add_executable(${EXAMPLE_NAME} src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE tie::tiny_inference_client spdlog::spdlog)
    install(TARGETS ${EXAMPLE_NAME} DESTINATION client/examples)
endfunction()

# find_package(OpenCV REQUIRED)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tiny_inference_client/examples)
find_package(spdlog REQUIRED)

add_example(client_example_server_info)
add_example(client_example_model_info)
add_example(client_example_model_infer)

# add_example(client_example_simple_inference)
# target_link_libraries(client_example_simple_inference
#     PRIVATE opencv::core
#     PRIVATE opencv::dnn
#     PRIVATE opencv::highgui
#     PRIVATE opencv::imgproc
#     PRIVATE opencv::imgcodecs)

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)
