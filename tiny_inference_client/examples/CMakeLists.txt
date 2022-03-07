function(add_example EXAMPLE_NAME)
    set(PROJECT_NAME ${EXAMPLE_NAME})
    add_executable(${PROJECT_NAME} src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE tie::client_core)
    install(TARGETS ${PROJECT_NAME} DESTINATION client/examples)
endfunction()

conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
find_package(OpenCV REQUIRED)

add_example(client_example_model_info)
add_example(client_example_simple_inference)
target_link_libraries(client_example_simple_inference
    PRIVATE opencv::core
    PRIVATE opencv::dnn
    PRIVATE opencv::highgui
    PRIVATE opencv::imgproc
    PRIVATE opencv::imgcodecs)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)
