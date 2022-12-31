add_subdirectory(tie_engine)

if(TIE_ENGINE_BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(TIE_ENGINE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()