set(TARGET_NAME server)

# if(MSVC)
#     add_definitions(-D_WIN32_WINNT=0x600)
# endif()

conan_cmake_run(CONANFILE ../conanfile.py BUILD missing)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
# find_package(Threads REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)

include(grpc_common)
add_executable(${TARGET_NAME} server_main.cpp ${proto_src} ${grpc_src})
target_link_libraries(${TARGET_NAME} PRIVATE gRPC::grpc++)
target_include_directories(${TARGET_NAME} PRIVATE ${PROTO_OUT_DIR})
