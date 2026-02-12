cmake_minimum_required(VERSION 3.16)
project(MineClone LANGUAGES C CXX)  
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

set(OpenGL_GL_PREFERENCE GLVND)

add_library(glad STATIC src_glad/glad.c)
target_include_directories(glad PUBLIC include)

file(GLOB SRC_FILES src/*.cpp)

add_executable(MineClone ${SRC_FILES})  
target_include_directories(MineClone PRIVATE
    include
)

target_link_libraries(MineClone
    glad
    SDL2::SDL2
    OpenGL::GL
)

