include_directories(${CMAKE_CURRENT_LIST_DIR}/3rd-party)

# Testing
aux_source_directory(./test_basic_config BASIC_SRC)
aux_source_directory(./test_json JSON_SRC)
add_executable(${PROJECT_NAME}_test main.cpp ${BASIC_SRC} ${JSON_SRC})

add_test(NAME ${PROJECT_NAME}_test
    COMMAND ${PROJECT_NAME}_test
    WORKING_DIRECTORY ${PROJECT_ROOT_DIR}
)

# Code Coverage Configuration
option(ENABLE_COVERAGE "Enable coverage build." OFF)
if(ENABLE_COVERAGE)
    message("Build with coverage infos.")
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
    find_package(codecov)
    add_coverage(${PROJECT_NAME}_test)
endif()
