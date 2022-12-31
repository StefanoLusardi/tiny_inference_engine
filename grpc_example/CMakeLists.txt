set(TARGET_NAME grpc_example)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/grpc_example)

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

add_executable(server src/server.cpp ${GRPC_PROTO_FILES})
target_compile_features(server PUBLIC cxx_std_17)
target_include_directories(server PRIVATE ${PROTO_OUT_DIR})
target_link_libraries(server PRIVATE gRPC::grpc++)

add_executable(client src/client.cpp ${GRPC_PROTO_FILES})
target_compile_features(client PUBLIC cxx_std_17)
target_include_directories(client PRIVATE ${PROTO_OUT_DIR})
target_link_libraries(client PRIVATE gRPC::grpc++)