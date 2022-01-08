cmake_minimum_required (VERSION 3.16)

project(grpc_client_server 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "gRPC Client/Server C++ examples"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/grpc_client_server"
)

set(CMAKE_VERBOSE_MAKEFILE ON)

message("System Info: " ${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CONAN_SYSTEM_INCLUDES ON)
set(CONAN_CMAKE_SILENT_OUTPUT ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

include(GNUInstallDirs)

option(BUILD_CLIENT "Build Tiny Inference Engine Client" ON)
option(BUILD_SERVER "Build Tiny Inference Engine Server" ON)

if(${BUILD_CLIENT})
    add_subdirectory(client)
endif()

if(${BUILD_SERVER})
    add_subdirectory(server)
endif()
