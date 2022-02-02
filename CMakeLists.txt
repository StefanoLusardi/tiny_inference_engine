cmake_minimum_required (VERSION 3.16)

project(tiny_inference_engine 
    LANGUAGES CXX 
    VERSION 1.0.0
    DESCRIPTION "tiny inference engine"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/tiny_inference_engine"
)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CONAN_SYSTEM_INCLUDES ON)
set(CONAN_CMAKE_SILENT_OUTPUT ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake" TLS_VERIFY ON)
endif()
include(${CMAKE_BINARY_DIR}/conan.cmake)

include(GNUInstallDirs)
include(CMakeDependentOption)

option(TIE_BUILD_CLIENT "Build Tiny Inference Engine Client" ON)
cmake_dependent_option(TIE_BUILD_CLIENT_EXAMPLES "Build Client Examples" ON "TIE_BUILD_CLIENT" OFF)
cmake_dependent_option(TIE_BUILD_CLIENT_UNIT_TESTS "Build Client Unit Tests" OFF "TIE_BUILD_CLIENT" OFF)

option(TIE_BUILD_SERVER "Build Tiny Inference Engine Server" ON)
cmake_dependent_option(TIE_BUILD_SERVER_EXAMPLES "Build Client Examples" ON "TIE_BUILD_CLIENT" OFF)
cmake_dependent_option(TIE_BUILD_SERVER_UNIT_TESTS "Build Server Unit Tests" OFF "TIE_BUILD_SERVER" OFF)

if(${TIE_BUILD_CLIENT})
    add_subdirectory(tiny_inference_client)
endif()

if(${TIE_BUILD_SERVER})
    add_subdirectory(tiny_inference_server)
endif()
