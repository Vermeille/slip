cmake_minimum_required(VERSION 2.6)
SET(CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -march=native -O3")
include_directories(parcxx/src)

set(sources src/parser.h)
include_directories(src)
add_subdirectory(tests)
