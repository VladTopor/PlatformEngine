cmake_minimum_required(VERSION 3.14)
project(perpg)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "/home/vladislav/CLionProjects/perpg/build/")

set(CMAKE_CXX_STANDARD 14)

include_directories(/usr/local/include/LuaCpp)
include_directories(/usr/include/lua5.3/)
include_directories(src/sfeMovie/include)
include_directories(src/)
link_directories(src/sfeMovie/bin)
add_library(logger src/logger.cpp src/logger.h)
add_executable(perpg src/main.cpp src/multiplayer.cpp src/multiplayer.h src/object.cpp src/object.h src/logger.cpp src/logger.h)

target_link_libraries(perpg logger)
target_link_libraries(perpg sfml-graphics)
target_link_libraries(perpg sfml-window)
target_link_libraries(perpg sfml-system)
target_link_libraries(perpg sfml-audio)
target_link_libraries(perpg sfml-network)
target_link_libraries(perpg luacpp)
target_link_libraries(perpg lua5.3)
target_link_libraries(perpg sfeMovie)