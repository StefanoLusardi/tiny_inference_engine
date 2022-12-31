add_subdirectory(tie_server)

if(TIE_SERVER_BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
