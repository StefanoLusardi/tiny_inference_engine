add_subdirectory(core)
add_subdirectory(server)

set(TIE_BUILD_SERVER_TESTS ON)
if(TIE_BUILD_SERVER_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()