cmake_minimum_required(VERSION 3.14)
project(flex-odbc-test VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GTest
find_package(GTest REQUIRED)

# Define source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable for the ITK test
add_executable(flex_odbc_test ${SOURCES})

# Link against the flex_odbc shared library and GTest
target_link_libraries(flex_odbc_test PRIVATE flex_odbc GTest::gtest_main odbc32 odbccp32)

# Include directories
target_include_directories(flex_odbc_test PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../sdk/include
    ${CMAKE_SOURCE_DIR}/../sdk/src
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(flex_odbc_test)

# Copy the flex_odbc.dll to the same output directory as the test runner
# This is crucial for runtime loading on Windows
add_custom_command(TARGET flex_odbc_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:flex_odbc>
    $<TARGET_FILE_DIR:flex_odbc_test>
    COMMENT "Copying flex_odbc.dll to test directory"
)
