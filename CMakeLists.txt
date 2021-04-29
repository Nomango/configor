cmake_minimum_required(VERSION 2.4)
project(jsonxx)

set(CMAKE_CXX_STANDARD 11)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_subdirectory(examples examples)
add_subdirectory(test test)
