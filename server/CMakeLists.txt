project(server 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "gRPC Server"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/grpc_client_server"
)

conan_cmake_run(CONANFILE conanfile.py BUILD missing)
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
    ${proto_src}
    ${grpc_src}
    engine/engine.cpp
    engine/parser.cpp
    engine/run.cpp
    engine/shutdown.cpp
    grpc_server/grpc_server.cpp
    http_server/http_server.cpp
    backend/onnx_backend.cpp
)

add_library(${TARGET_NAME} SHARED ${TARGET_SRC})

set(CUDA_PACKAGE_SUFFIX "")
set(ONNXRUNTIME_LIBS onnxruntime)
option(WITH_CUDA "Accelerate ONNX Runtime backend with CUDA GPU. Default is CPU." OFF)

if(WITH_CUDA AND NOT APPLE)
    set(CUDA_PACKAGE_SUFFIX "-gpu")
    list(ONNXRUNTIME_LIBS APPEND onnxruntime_providers_shared onnxruntime_providers_cuda)
endif()

if(WITH_CUDA AND APPLE)
    message(WARN "ONNX Runtime GPU acceleration is not supported on MacOS. Default to CPU.")
endif()

if(UNIX)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/linux${CUDA_PACKAGE_SUFFIX}/include)
    target_link_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/linux${CUDA_PACKAGE_SUFFIX}/lib)
elseif(WIN32)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/windows${CUDA_PACKAGE_SUFFIX}/include)
    target_link_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/windows${CUDA_PACKAGE_SUFFIX}/lib)
elseif(APPLE)
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/macos/include)
    target_link_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/onnxruntime/macos/lib)
endif()

# set(CMAKE_INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
# set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

target_include_directories(${TARGET_NAME} PRIVATE ${PROTO_OUT_DIR})
target_link_libraries(${TARGET_NAME}
    PRIVATE CLI11::CLI11
    PRIVATE spdlog::spdlog
    PRIVATE gRPC::grpc++
    PRIVATE ${ONNXRUNTIME_LIBS}
)

# include(sanitizers)
# add_sanitizers(engine)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE engine)
