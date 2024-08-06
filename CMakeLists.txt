cmake_minimum_required(VERSION 3.30)

project(Cloven)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows")
set(OpenGL_GL_PREFERENCE GLVND)

if(CMAKE_TOOLCHAIN_FILE)
  include(${CMAKE_TOOLCHAIN_FILE})
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

find_library(imgui REQUIRED HINTS "${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib" NAMES imgui)
link_directories("${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/lib")
include_directories("${CMAKE_SOURCE_DIR}/vcpkg_installed/x64-windows/include")

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

add_executable(Cloven ${SOURCES})
target_link_libraries(Cloven PRIVATE ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} glfw ${GLM_LIBRARIES} imgui)

file(GLOB_RECURSE SHADER_FILES "${CMAKE_SOURCE_DIR}/shaders/*.vert" "${CMAKE_SOURCE_DIR}/shaders/*.frag")
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    DEPENDS ${SHADER_FILES}
)
add_dependencies(${PROJECT_NAME} copy_shaders)
