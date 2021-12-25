project(server 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "gRPC Server"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/grpc_client_server"
)

conan_cmake_run(CONANFILE conanfile.py BUILD missing)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

find_package(CLI11 REQUIRED)
find_package(spdlog REQUIRED)

set(engine_src
    ${proto_src}
    ${grpc_src}
    engine/engine.cpp
    engine/parser.cpp
    engine/run.cpp
    engine/shutdown.cpp
    grpc_server/grpc_server.cpp
    http_server/http_server.cpp
)

add_library(engine ${engine_src})
target_link_libraries(engine
    PRIVATE CLI11::CLI11
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::grpc++)
target_include_directories(engine PRIVATE ${PROTO_OUT_DIR})

# include(sanitizers)
# add_sanitizers(engine)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE engine)
