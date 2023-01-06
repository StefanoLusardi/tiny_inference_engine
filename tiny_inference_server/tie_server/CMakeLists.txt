set(TARGET_NAME tiny_inference_server)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tiny_inference_server/tie_server)

find_package(CLI11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

file(STRINGS "VERSION" TIE_SERVER_VERSION)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version.hpp)

set(TARGET_SRC
    ${GRPC_PROTO_FILES}
    src/version.hpp

    src/command_line_parser.cpp
    src/command_line_parser.hpp
    src/server_config.hpp
    src/server_interface.hpp
    src/server_manager.hpp
    src/server_manager.cpp
    src/grpc_server.hpp
    src/grpc_server.cpp
    src/http_server.hpp
    src/http_server.cpp
    src/server_shutdown.hpp
    src/server_shutdown.cpp
    src/logging.hpp
    src/logging.cpp
    src/main.cpp

    src/rpc/rpc.hpp
    src/rpc/rpc_io.hpp
    src/rpc/rpc_pool.hpp
    src/rpc/rpc_pool.cpp
    src/rpc/rpc_unary_async.hpp
    src/rpc/rpc_unary_async.cpp

    src/rpc/rpc_server_live.hpp
    src/rpc/rpc_server_live.cpp
    src/rpc/rpc_server_metadata.hpp
    src/rpc/rpc_server_metadata.cpp
    src/rpc/rpc_server_ready.hpp
    src/rpc/rpc_server_ready.cpp

    src/rpc/rpc_model_infer.hpp
    src/rpc/rpc_model_infer.cpp
    src/rpc/rpc_model_list.hpp
    src/rpc/rpc_model_list.cpp
    src/rpc/rpc_model_load.hpp
    src/rpc/rpc_model_load.cpp
    src/rpc/rpc_model_metadata.hpp
    src/rpc/rpc_model_metadata.cpp
    src/rpc/rpc_model_ready.hpp
    src/rpc/rpc_model_ready.cpp
    src/rpc/rpc_model_unload.hpp
    src/rpc/rpc_model_unload.cpp
)

add_executable(${TARGET_NAME})

target_sources(${TARGET_NAME} PRIVATE ${TARGET_SRC})

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

target_include_directories(${TARGET_NAME} PRIVATE ${PROTO_OUT_DIR})

target_link_libraries(${TARGET_NAME}
    PRIVATE CLI11::CLI11
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::grpc++
    PRIVATE tie::tiny_inference_engine
)

target_precompile_headers(${TARGET_NAME} 
    PRIVATE <CLI/CLI.hpp>
    PRIVATE <spdlog/spdlog.h>
    PRIVATE <spdlog/sinks/basic_file_sink.h>
    PRIVATE <spdlog/sinks/stdout_color_sinks.h>
    PRIVATE <services.grpc.pb.h>
    PRIVATE <grpcpp/grpcpp.h>
    PRIVATE ${PROTO_INCLUDE}
    PRIVATE ${GRPC_INCLUDE}
)

if(TIE_SERVER_ENABLE_SANITIZERS)
    include(sanitizers)
    add_sanitizers(${TARGET_NAME})
endif()

install(TARGETS ${TARGET_NAME} DESTINATION tiny_inference_server/server)
