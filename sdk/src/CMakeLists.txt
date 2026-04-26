# Generate protobuf files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/schema/driver.proto)

# Define source files
set(SOURCES
    odbc_api.cpp
    config_reader.cpp
    process_launcher.cpp
    pipe_ipc_channel.cpp
    flex_odbc.def
    ${PROTO_SRCS}
)

# Create the shared library
add_library(flex_odbc SHARED ${SOURCES})

# Target includes
target_include_directories(flex_odbc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(flex_odbc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}) # For generated proto headers

# Link libraries
target_link_libraries(flex_odbc PRIVATE protobuf::libprotobuf)

# Windows specific configurations
if(WIN32)
    set_target_properties(flex_odbc PROPERTIES PREFIX "" SUFFIX ".dll")
    target_link_libraries(flex_odbc PRIVATE odbc32 odbccp32)
endif()
