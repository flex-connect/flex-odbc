cmake_minimum_required(VERSION 3.14)

# Add the executable for the SDK tests
add_executable(sdk_test_runner src/test_main.cpp)

# Link against the flex_odbc shared library
target_link_libraries(sdk_test_runner PRIVATE flex_odbc)

# Include directories for the SDK headers
target_include_directories(sdk_test_runner PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Add the test to CTest
add_test(NAME SdkTests COMMAND sdk_test_runner)

# Copy the flex_odbc.dll to the same output directory as the test runner
# This is crucial for runtime loading on Windows
add_custom_command(TARGET sdk_test_runner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:flex_odbc>
    $<TARGET_FILE_DIR:sdk_test_runner>
    COMMENT "Copying flex_odbc.dll to test runner directory"
)

