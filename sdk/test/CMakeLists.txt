cmake_minimum_required(VERSION 3.14)

# Find GTest
find_package(GTest REQUIRED)

# Add the executable for the SDK tests
add_executable(sdk_test_runner test_main.cpp)

# Link against the flex_odbc shared library and GTest
target_link_libraries(sdk_test_runner PRIVATE flex_odbc GTest::gtest_main)

# Include directories for the SDK headers
target_include_directories(sdk_test_runner PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src # For generated proto headers
)

# Add the test to CTest
include(GoogleTest)
gtest_discover_tests(sdk_test_runner)

# Copy the flex_odbc.dll to the same output directory as the test runner
# This is crucial for runtime loading on Windows
add_custom_command(TARGET sdk_test_runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:flex_odbc>
    $<TARGET_FILE_DIR:sdk_test_runner>
    COMMENT "Copying flex_odbc.dll to test runner directory"
)
