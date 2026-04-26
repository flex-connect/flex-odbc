cmake_minimum_required(VERSION 3.14)
project(flex-odbc VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Protobuf
find_package(Protobuf REQUIRED)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(src)

# Testing Setup
enable_testing()

