# GTest
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Testing
aux_source_directory(. SOURCES)

add_executable(${PROJECT_NAME}_test ${SOURCES})
target_link_libraries(${PROJECT_NAME}_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES} pthread)
add_test(NAME ${PROJECT_NAME}_test COMMAND ${PROJECT_NAME}_test WORKING_DIRECTORY ${PROJECT_ROOT_DIR})

# Code Coverage Configuration
option(ENABLE_COVERAGE "Enable coverage build." OFF)
if(ENABLE_COVERAGE)
    message("Build with coverage infos.")
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/cmake" ${CMAKE_MODULE_PATH})
    find_package(codecov)
    add_coverage(${PROJECT_NAME}_test)
endif()
