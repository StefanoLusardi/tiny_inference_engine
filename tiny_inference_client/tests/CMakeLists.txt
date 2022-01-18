set(TARGET_NAME tiny_inference_client_tests)

conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS)# OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(GTest COMPONENTS GTest REQUIRED)

add_executable(${TARGET_NAME}
	src/test_main.cpp
	src/engine_parser_test.cpp
)

# target_include_directories(${TARGET_NAME} PRIVATE include)
target_link_libraries(${TARGET_NAME} 
	PRIVATE tie::client_core 
	PRIVATE GTest::GTest
)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})

# include(sanitizers)
# add_sanitizers(${TARGET_NAME})

install(TARGETS ${TARGET_NAME} DESTINATION client/client_tests)