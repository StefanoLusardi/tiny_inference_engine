add_subdirectory(client_core)

set(TIE_BUILD_CLIENT_TESTS ON)
if(TIE_BUILD_CLIENT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

set(TIE_BUILD_CLIENT_EXAMPLES ON)
if(TIE_BUILD_CLIENT_EXAMPLES)
    add_subdirectory(examples)
endif()