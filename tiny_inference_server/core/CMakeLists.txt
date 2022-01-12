# project(tiny_inference_server 
#     LANGUAGES CXX 
#     VERSION 1.0.0
#     DESCRIPTION "tiny inference server"
#     HOMEPAGE_URL "https://github.com/StefanoLusardi/tiny_inference_engine"
# )

conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS)# OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(CLI11 REQUIRED)
find_package(spdlog REQUIRED)
find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${CMAKE_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

set(TARGET_NAME engine)
set(TARGET_SRC
    engine/include/run.hpp
    engine/run.cpp
    engine/parser.hpp
    engine/parser.cpp
    engine/shutdown.hpp
    engine/shutdown.cpp
    engine/engine.hpp
    engine/engine.cpp
    engine/server_manager.hpp

    backend/backend_interface.hpp
    backend/infer_request.hpp
    backend/infer_response.hpp
    backend/onnx_backend.hpp
    backend/onnx_backend.cpp

    ${GRPC_PROTO_FILES}
    server/server_interface.hpp
    server/grpc_server.hpp
    server/grpc_server.cpp
    server/http_server.hpp
    server/http_server.cpp
)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_library(${TARGET_NAME} SHARED ${TARGET_SRC})
add_library(tie::${TARGET_NAME} ALIAS ${TARGET_NAME})

set(CUDA_PACKAGE_SUFFIX "")
set(ONNXRUNTIME_LIBS onnxruntime)
option(WITH_CUDA "Accelerate ONNX Runtime backend with CUDA on Nvidia GPU. Default is CPU." OFF)

if(WITH_CUDA AND NOT APPLE)
    set(CUDA_PACKAGE_SUFFIX "-gpu")
    list(ONNXRUNTIME_LIBS APPEND onnxruntime_providers_shared onnxruntime_providers_cuda)
endif()

if(WITH_CUDA AND APPLE)
    message(WARN "ONNX Runtime GPU acceleration is not supported on MacOS. Default to CPU.")
endif()

if(WIN32)
    set(PLATFORM "windows")
elseif(UNIX AND NOT APPLE)
    set(PLATFORM "linux")
elseif(APPLE)
    set(PLATFORM "macos")
endif()
    
set(ONNXRUNTIME_DIR ${CMAKE_SOURCE_DIR}/onnxruntime/${PLATFORM}${CUDA_PACKAGE_SUFFIX})

target_link_directories(${TARGET_NAME} 
    PRIVATE ${ONNXRUNTIME_DIR}/lib
)

target_include_directories(${TARGET_NAME}
    PUBLIC engine/include
    PRIVATE ${PROTO_OUT_DIR}
    PRIVATE ${ONNXRUNTIME_DIR}/include
)

target_link_libraries(${TARGET_NAME}
    PRIVATE CLI11::CLI11
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::grpc++
    PRIVATE ${ONNXRUNTIME_LIBS}
)

# include(sanitizers)
# add_sanitizers(engine)

install(TARGETS engine DESTINATION .)
install(DIRECTORY ${ONNXRUNTIME_DIR}/lib/ DESTINATION .)