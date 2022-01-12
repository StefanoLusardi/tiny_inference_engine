# project(tiny_inference_server 
#     LANGUAGES CXX 
#     VERSION 1.0.0
#     DESCRIPTION "tiny inference server"
#     HOMEPAGE_URL "https://github.com/StefanoLusardi/tiny_inference_engine"
# )

set(PROJECT_NAME tiny_inference_server)
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE tie::engine)

install(TARGETS ${PROJECT_NAME} DESTINATION .)
