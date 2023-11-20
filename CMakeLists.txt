cmake_minimum_required(VERSION 3.26)
project(cg)

set(CMAKE_CXX_STANDARD 23)

# Disable building GLFW docs, tests, and examples
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)
include_directories(glad/include glfw/deps)

find_package(OpenGL REQUIRED)

add_executable(cg src/main.cpp glad/src/gl.c)
target_link_libraries(cg OpenGL::GL glfw)

configure_file(shaders/FragmentShaderStart.glsl shaders/FragmentShaderStart.glsl COPYONLY)
configure_file(shaders/VertexShaderStart.glsl shaders/VertexShaderStart.glsl COPYONLY)
configure_file(shaders/DiffuseLightComponent.vert shaders/DiffuseLightComponent.vert COPYONLY)
configure_file(shaders/DiffuseLightComponent.frag shaders/DiffuseLightComponent.frag COPYONLY)
