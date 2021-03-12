cmake_minimum_required(VERSION 2.4)
set(CMAKE_CXX_STANDARD 11)
project(jsonxx)

# GTest
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Testing
include_directories(${CMAKE_CURRENT_LIST_DIR}/../include)

set(TESTING_SOURCES
    test_basic_json.cpp
    test_parser.cpp
    test_serializer.cpp
    test_utils.cpp
)

add_executable(${PROJECT_NAME}_test ${TESTING_SOURCES})
target_link_libraries(${PROJECT_NAME}_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
add_test(${PROJECT_NAME}_test ${PROJECT_NAME}_test)
