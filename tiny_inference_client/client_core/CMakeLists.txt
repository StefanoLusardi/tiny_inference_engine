set(TARGET_NAME tiny_inference_client_core)

conan_cmake_run(CONANFILE conanfile.py BUILD missing KEEP_RPATHS)# OUTPUT_QUIET)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})

find_package(gRPC REQUIRED)
find_package(Protobuf REQUIRED)

set(PROTO_FILENAME services)
set(PROTO_FILE ${PROTO_FILENAME}.proto)
set(PROTO_IN_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROTO_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/proto)
include(grpc_common)

set(TARGET_SRC
    include/client.hpp
    src/client.cpp
    ${GRPC_PROTO_FILES}
)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_library(${TARGET_NAME} ${TARGET_SRC})
add_library(tie::client_core ALIAS ${TARGET_NAME})

target_include_directories(${TARGET_NAME}
    PUBLIC include
    PRIVATE ${PROTO_OUT_DIR}
)

target_link_libraries(${TARGET_NAME} PRIVATE gRPC::grpc++)

# include(sanitizers)
# add_sanitizers(${TARGET_NAME})

# install(TARGETS ${TARGET_NAME} DESTINATION .)