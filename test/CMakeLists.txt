cmake_minimum_required(VERSION 3.14)

add_executable(test_runner src/main.cpp)

# Link against the flex_odbc shared library and whatever testing framework you use (e.g., GTest)
target_link_libraries(test_runner PRIVATE flex_odbc)

# Include directories if necessary
target_include_directories(test_runner PRIVATE ${CMAKE_SOURCE_DIR}/sdk/src)

add_test(NAME IntegrationTests COMMAND test_runner)
