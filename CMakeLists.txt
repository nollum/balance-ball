cmake_minimum_required(VERSION 3.1)

project(BalanceBall VERSION 1.0 DESCRIPTION "Balance the ball" LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB SHADERS "shaders/*")

add_executable(game ${SOURCES} ${SHADERS})

target_include_directories(game PUBLIC include)

target_link_libraries(game glfw3 GL X11 pthread Xrandr Xi dl m)
