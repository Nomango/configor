cmake_minimum_required(VERSION 2.4)
set(CMAKE_CXX_STANDARD 11)
project(jsonxx)

include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

set(EXAMPLE_SOURCES
    main.cpp
    user_info.cpp
)

add_executable(${PROJECT_NAME}_example ${EXAMPLE_SOURCES})
