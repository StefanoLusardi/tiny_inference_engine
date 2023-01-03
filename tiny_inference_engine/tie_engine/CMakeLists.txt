set(TARGET_NAME tiny_inference_engine)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/modules/tiny_inference_engine/tie_engine)

find_package(spdlog REQUIRED)

set(TARGET_SRC_PUBLIC
    include/tie_engine/infer_request.hpp
    include/tie_engine/infer_response.hpp
    include/tie_engine/model_metadata.hpp

    include/tie_engine/backend_factory.hpp
    include/tie_engine/backend_interface.hpp
    include/tie_engine/backend_type.hpp

    include/tie_engine/engine_factory.hpp
    include/tie_engine/engine_interface.hpp
)

set(TARGET_SRC_PRIVATE
    src/backend_factory.cpp
    src/engine_factory.cpp
    src/engine.hpp
    src/engine.cpp
    src/null_backend/null_backend.hpp
    src/null_backend/null_backend.cpp
)

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif()

add_library(${TARGET_NAME} SHARED)
add_library(tie::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_sources(${TARGET_NAME} 
    PUBLIC ${TARGET_SRC_PUBLIC}
    PRIVATE ${TARGET_SRC_PRIVATE}
)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_17)

target_include_directories(${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:include>
)

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
    
# set(ONNXRUNTIME_DIR ${CMAKE_SOURCE_DIR}/tiny_inference_server/onnxruntime/${PLATFORM}${CUDA_PACKAGE_SUFFIX})
# target_link_directories(${TARGET_NAME} 
#     PRIVATE ${ONNXRUNTIME_DIR}/lib
# )

target_include_directories(${TARGET_NAME}
    PUBLIC include/engine
    # PRIVATE ${ONNXRUNTIME_DIR}/include
)

target_link_libraries(${TARGET_NAME}
    PRIVATE spdlog::spdlog
    # PRIVATE ${ONNXRUNTIME_LIBS}
)

if(TIE_ENGINE_ENABLE_SANITIZERS)
    include(sanitizers)
    add_sanitizers(${TARGET_NAME})
endif()

install(TARGETS ${TARGET_NAME} DESTINATION tiny_inference_engine/engine)
# install(DIRECTORY ${ONNXRUNTIME_DIR}/lib/ DESTINATION server)