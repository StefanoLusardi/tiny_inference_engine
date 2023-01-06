cmake_minimum_required (VERSION 3.16)

project(tiny_inference_engine 
    LANGUAGES CXX
    DESCRIPTION "tiny inference engine"
    HOMEPAGE_URL "https://github.com/StefanoLusardi/tiny_inference_engine"
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(CMakeDependentOption)

message("Compiler:  ${CMAKE_CXX_COMPILER_ID} - Version: ${CMAKE_CXX_COMPILER_VERSION}")

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    list(APPEND CXX_FLAGS "-fPIC" "-Wall" "-fno-rtti" "-fno-exceptions")
    list(APPEND CXX_FLAGS_DEBUG "-Wsuggest-final-types" "-Wsuggest-final-methods" "-Wsuggest-override")
    list(APPEND CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    list(APPEND CXX_FLAGS "-fPIC" "-Wall" "-fno-rtti" "-fno-exceptions" "-Qunused-arguments" "-fcolor-diagnostics")
    list(APPEND CXX_FLAGS_DEBUG "-Wdocumentation -fstandalone-debug")
    list(APPEND CXX_FLAGS_RELEASE "-O" "-Wno-unused")
endif()

option(TIE_CLIENT_BUILD "Build Client" True)
cmake_dependent_option(TIE_CLIENT_BUILD_EXAMPLES "Build Client examples" True TIE_CLIENT_BUILD ON)
cmake_dependent_option(TIE_CLIENT_BUILD_UNIT_TESTS "Build Client unit tests" False TIE_CLIENT_BUILD ON)
cmake_dependent_option(TIE_CLIENT_ENABLE_SANITIZERS "Enable sanitizers for Client" False TIE_CLIENT_BUILD ON)
cmake_dependent_option(TIE_CLIENT_ENABLE_LOG "Enable internal logger for Client" True TIE_CLIENT_BUILD ON)

option(TIE_SERVER_BUILD "Build Server" True)
cmake_dependent_option(TIE_SERVER_BUILD_EXAMPLES "Build Server examples" False TIE_SERVER_BUILD ON)
cmake_dependent_option(TIE_SERVER_BUILD_UNIT_TESTS "Build Server unit tests" False TIE_SERVER_BUILD ON)
cmake_dependent_option(TIE_SERVER_ENABLE_SANITIZERS "Enable sanitizers build for Server" False TIE_SERVER_BUILD ON)

cmake_dependent_option(TIE_ENGINE_BUILD "Build Engine" True TIE_SERVER_BUILD OFF)
cmake_dependent_option(TIE_ENGINE_BUILD_EXAMPLES "Build Engine examples" False TIE_ENGINE_BUILD ON)
cmake_dependent_option(TIE_ENGINE_BUILD_UNIT_TESTS "Build Engine unit tests" False TIE_ENGINE_BUILD ON)
cmake_dependent_option(TIE_ENGINE_ENABLE_SANITIZERS "Enable sanitizers build for Engine" False TIE_ENGINE_BUILD ON)

if(${TIE_ENGINE_BUILD})
    add_subdirectory(tiny_inference_engine)
endif()

if(${TIE_SERVER_BUILD})
    add_subdirectory(tiny_inference_server)
endif()

if(${TIE_CLIENT_BUILD})
    add_subdirectory(tiny_inference_client)
endif()
