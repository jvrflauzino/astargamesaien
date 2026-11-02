cmake_minimum_required(VERSION 3.15)
project(fruitbox_solver CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(src)

add_library(fruitbox_core
    src/board.cpp
)

target_link_libraries(test_core PRIVATE fruitbox_core)
