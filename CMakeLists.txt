cmake_minimum_required(VERSION 3.26)
project(FacialExps)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES main.cpp glad.c)

include_directories(${PROJECT_SOURCE_DIR}/include)

configure_file(${CMAKE_SOURCE_DIR}/shaders/shader.vs ${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.vs COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/shaders/shader.fs ${CMAKE_CURRENT_BINARY_DIR}/shaders/shader.fs COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/data/cube.obj ${CMAKE_CURRENT_BINARY_DIR}/data/cube.obj COPYONLY)

add_executable(FacialExps ${SOURCE_FILES})

target_link_libraries(FacialExps glfw)
