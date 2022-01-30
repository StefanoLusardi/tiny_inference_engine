add_subdirectory(server_core)
add_subdirectory(server)

if(TIE_BUILD_SERVER_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()