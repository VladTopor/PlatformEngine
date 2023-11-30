cmake_minimum_required(VERSION 3.14)
project(perpg)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/vladislav/CLionProjects/perpg/build/")

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include/LuaCpp)
include_directories(/usr/include/lua5.3/)

add_executable(perpg src/main.cpp src/multiplayer.cpp src/multiplayer.h)

target_link_libraries(perpg sfml-graphics)
target_link_libraries(perpg sfml-window)
target_link_libraries(perpg sfml-system)
target_link_libraries(perpg sfml-audio)
target_link_libraries(perpg sfml-network)
target_link_libraries(perpg luacpp)
target_link_libraries(perpg lua5.3)