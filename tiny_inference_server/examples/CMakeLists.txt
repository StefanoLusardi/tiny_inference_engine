function(add_example EXAMPLE_NAME)
    set(PROJECT_NAME ${EXAMPLE_NAME})
    add_executable(${PROJECT_NAME} src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${PROJECT_NAME} 
        PRIVATE tie::server_core
        PRIVATE opencv::core
        PRIVATE opencv::dnn
        PRIVATE opencv::highgui
        PRIVATE opencv::imgproc
        PRIVATE opencv::imgcodecs)
    install(TARGETS ${PROJECT_NAME} DESTINATION server/examples)
endfunction()

conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
find_package(OpenCV REQUIRED)

add_example(server_backend_runner)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/images/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/images)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/labels/synset.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/labels)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/squeezenet1.1-7.onnx DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)