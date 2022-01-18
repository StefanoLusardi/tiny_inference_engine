function(add_example EXAMPLE_NAME)
    set(PROJECT_NAME ${EXAMPLE_NAME})
    add_executable(${PROJECT_NAME} src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE tie::client_core)
    install(TARGETS ${PROJECT_NAME} DESTINATION client/examples)
endfunction()

add_example(client_example_model_info)
add_example(client_example_simple_inference)

