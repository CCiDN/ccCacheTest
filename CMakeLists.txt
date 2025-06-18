cmake_minimum_required(VERSION 3.0)

project(lruCaCheTest)

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

add_executable(TestMyCache testMain.cpp src/LfuCache.tpp)