cmake_minimum_required(VERSION 2.4)
set(CMAKE_CXX_STANDARD 17)
project(jsonxx)

# GTest
find_package(GTEST REQUIRED)
enable_testing()
include_directories(${GTEST_INCLUDE_DIR})

# Testing
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

set(TESTING_SOURCES
    test_main.cpp
    test_basic_json.cpp
    test_parser.cpp
    test_serializer.cpp
)

add_executable(${PROJECT_NAME}_test ${TESTING_SOURCES})
add_dependencies(${PROJECT_NAME}_test GTest)
target_link_libraries(${PROJECT_NAME}_test ${GTEST_LIBRARIES})
add_test(${PROJECT_NAME}_test ${PROJECT_NAME}_test)
