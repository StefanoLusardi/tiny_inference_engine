function(add_example EXAMPLE_NAME)
    set(PROJECT_NAME ${EXAMPLE_NAME})
    add_executable(${PROJECT_NAME} src/${EXAMPLE_NAME}.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE tie::server_core)
    install(TARGETS ${PROJECT_NAME} DESTINATION server/examples)
endfunction()

add_example(server_backend_runner)
