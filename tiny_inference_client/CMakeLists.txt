add_subdirectory(client_core)

if(TIE_BUILD_CLIENT_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(TIE_BUILD_CLIENT_EXAMPLES)
    add_subdirectory(examples)
endif()