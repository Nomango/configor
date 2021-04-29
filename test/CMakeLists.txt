# GTest
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIR})

# Testing
aux_source_directory(. SOURCES)

add_executable(${PROJECT_NAME}_test ${SOURCES})
target_link_libraries(${PROJECT_NAME}_test ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
add_test(${PROJECT_NAME}_test ${PROJECT_NAME}_test)

