set(TARGET_NAME server_tests)

conan_cmake_configure(
    GENERATORS cmake_find_package
    REQUIRES gtest/1.10.0
    OPTIONS gtest:no_main=True
    OUTPUT_QUIET
)
conan_cmake_autodetect(conan_settings)
conan_cmake_install(
    PATH_OR_REFERENCE .
    BUILD missing
    SETTINGS ${conan_settings}
    OUTPUT_QUIET
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/tests)

find_package(GTest COMPONENTS GTest REQUIRED)

set(VCPP_TEST_SOURCES
)

set(VCPP_TEST_HEADERS
)

add_executable(${TARGET_NAME}
	test.cpp
	${VCPP_TEST_HEADERS}
	${VCPP_TEST_SOURCES}
)

target_link_libraries(${TARGET_NAME} 
	PRIVATE video_capture 
	PRIVATE GTest::GTest
)

target_include_directories(${TARGET_NAME} PRIVATE include)

include(GoogleTest)
gtest_add_tests(TARGET ${TARGET_NAME})
# gtest_discover_tests(${TARGET_NAME})
