conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS)# OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

set(TARGET_NAME server_tests)

find_package(GTest COMPONENTS GTest REQUIRED)

add_executable(${TARGET_NAME}
	test.cpp
)

target_link_libraries(${TARGET_NAME} 
	PRIVATE engine 
	PRIVATE GTest::GTest
)

# target_include_directories(${TARGET_NAME} PRIVATE include)

include(GoogleTest)
# gtest_add_tests(TARGET ${TARGET_NAME})
gtest_discover_tests(${TARGET_NAME})
