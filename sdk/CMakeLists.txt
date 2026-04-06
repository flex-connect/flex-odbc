cmake_minimum_required(VERSION 3.14)
project(flex-odbc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Formatting enforcement
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -i -style=file ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/test/*.cpp
    )
endif()

# Build the Shared Library (ODBC Driver)
add_library(flex_odbc SHARED src/odbc_api.cpp src/flex_odbc.def)

# Output .dll on Windows
if(WIN32)
    set_target_properties(flex_odbc PROPERTIES PREFIX "" SUFFIX ".dll")
    # Link against ODBC libraries
    target_link_libraries(flex_odbc PRIVATE odbc32 odbccp32)
endif()

# Target includes & Protobuf dependencies would go here
# find_package(Protobuf REQUIRED)
# target_link_libraries(flex_odbc PRIVATE protobuf::libprotobuf)

# Testing Setup
enable_testing()
add_subdirectory(test)

