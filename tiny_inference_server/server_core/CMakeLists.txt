set(TARGET_NAME tiny_inference_server_core)

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

set(TARGET_SRC
    engine/include/run.hpp
    engine/run.cpp
    engine/parser.hpp
    engine/parser.cpp
    engine/shutdown.hpp
    engine/shutdown.cpp
    engine/engine.hpp
    engine/engine.cpp

    backend/include/backend_interface.hpp
    backend/include/infer_request.hpp
    backend/include/infer_response.hpp
    backend/include/backend_factory.hpp
    backend/backend_factory.cpp
    backend/onnx_backend.hpp
    backend/onnx_backend.cpp
    backend/pytorch_backend.hpp
    backend/pytorch_backend.cpp
    backend/null_backend.hpp
    backend/null_backend.cpp

    ${GRPC_PROTO_FILES}
    server/server_interface.hpp
    server/server_manager.hpp
    server/server_manager.cpp
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
add_library(tie::server_core ALIAS ${TARGET_NAME})

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
    
set(ONNXRUNTIME_DIR ${CMAKE_SOURCE_DIR}/tiny_inference_server/onnxruntime/${PLATFORM}${CUDA_PACKAGE_SUFFIX})

target_link_directories(${TARGET_NAME} 
    PRIVATE ${ONNXRUNTIME_DIR}/lib
)

target_include_directories(${TARGET_NAME}
    PUBLIC engine/include
    PUBLIC backend/include
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

install(TARGETS ${TARGET_NAME} DESTINATION server)
install(DIRECTORY ${ONNXRUNTIME_DIR}/lib/ DESTINATION server)