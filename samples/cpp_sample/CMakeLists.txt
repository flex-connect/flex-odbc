cmake_minimum_required(VERSION 3.10)
project(flex_odbc_cpp_sample CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/src/flex_odbc.proto)

add_executable(flex_odbc_cpp_sidecar src/main.cpp ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(flex_odbc_cpp_sidecar PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${PROTOBUF_INCLUDE_DIRS}
)

target_link_libraries(flex_odbc_cpp_sidecar PRIVATE
    ${PROTOBUF_LIBRARIES}
)

install(TARGETS flex_odbc_cpp_sidecar DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/driver.properties DESTINATION bin)
