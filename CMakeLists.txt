cmake_minimum_required(VERSION 3.16)
project(MineClone LANGUAGES C CXX)  

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)

add_library(glad STATIC src_glad/glad.c)
target_include_directories(glad PUBLIC include)
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl2.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)
target_link_libraries(imgui PUBLIC
    SDL2::SDL2
    OpenGL::GL
)

file(GLOB SRC_FILES src/*.cpp)

add_executable(MineClone ${SRC_FILES})  
target_include_directories(MineClone PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui          
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(MineClone
    glad
    imgui
    SDL2::SDL2
    OpenGL::GL
)

