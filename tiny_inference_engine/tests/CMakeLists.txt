set(TARGET_NAME tiny_inference_server_tests)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/${TARGET_NAME})

find_package(GTest)
include(GoogleTest)

add_executable(${TARGET_NAME}
	src/main.cpp
)

# target_include_directories(${TARGET_NAME} PRIVATE include)
target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

target_link_libraries(${TARGET_NAME} 
	PRIVATE tie::server_core 
	PRIVATE GTest::GTest
)

gtest_discover_tests(${TARGET_NAME})

# include(sanitizers)
# add_sanitizers(${TARGET_NAME})

install(TARGETS ${TARGET_NAME} DESTINATION server)
