set(TARGET_NAME tiny_inference_client)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tiny_inference_client/tie_client)

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

set(TARGET_SRC_PUBLIC
    include/tie_client/client_interface.hpp
    include/tie_client/client_factory.hpp
    include/tie_client/call_result.hpp
    include/tie_client/infer_request.hpp
    include/tie_client/infer_response.hpp
    include/tie_client/data_type.hpp
)

set(TARGET_SRC_PRIVATE
    src/data_type.cpp
    src/grpc_client.hpp
    src/grpc_client.cpp
    src/tensor_converter.hpp
    src/tensor_converter.cpp
    src/client_factory.cpp
    src/call_result.cpp
    ${GRPC_PROTO_FILES}
)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_library(${TARGET_NAME} STATIC)
add_library(tie::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_sources(${TARGET_NAME} 
    PUBLIC ${TARGET_SRC_PUBLIC}
    PRIVATE ${TARGET_SRC_PRIVATE}
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

if(TIE_CLIENT_ENABLE_LOG)
    target_compile_definitions(${TARGET_NAME} PRIVATE TIE_CLIENT_ENABLE_LOG)
endif()

target_include_directories(${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>
    PRIVATE ${PROTO_OUT_DIR}
)

target_link_libraries(${TARGET_NAME} 
    PRIVATE gRPC::grpc++
    PRIVATE spdlog::spdlog
)

target_precompile_headers(${TARGET_NAME}
    PRIVATE <services.grpc.pb.h>
    PRIVATE <grpcpp/grpcpp.h>
    PRIVATE ${PROTO_INCLUDE}
    PRIVATE ${GRPC_INCLUDE}
)

if(TIE_CLIENT_ENABLE_SANITIZERS)
    include(sanitizers)
    add_sanitizers(${TARGET_NAME})
endif()

install(TARGETS ${TARGET_NAME} DESTINATION tiny_inference_client/client)