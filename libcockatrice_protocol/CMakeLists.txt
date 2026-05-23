# Top-level wrapper for the protobuf library

# Single source of truth for the network protocol version. The same JSON file is
# shipped in the @cockatrice/protocol npm package so TypeScript consumers read
# the identical value at runtime. Regex-extracted (instead of string(JSON ...))
# so we keep the project's CMake 3.10 floor.
set(PROTOCOL_VERSION_JSON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/protocol_version.json")
file(READ "${PROTOCOL_VERSION_JSON_PATH}" PROTOCOL_VERSION_JSON)
string(REGEX MATCH "\"protocolVersion\"[ \t\r\n]*:[ \t\r\n]*([0-9]+)" _ "${PROTOCOL_VERSION_JSON}")
if(NOT CMAKE_MATCH_1)
  message(FATAL_ERROR "Failed to extract protocolVersion from ${PROTOCOL_VERSION_JSON_PATH}")
endif()
set(COCKATRICE_PROTOCOL_VERSION "${CMAKE_MATCH_1}")
set_property(
  DIRECTORY
  APPEND
  PROPERTY CMAKE_CONFIGURE_DEPENDS "${PROTOCOL_VERSION_JSON_PATH}"
)
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/protocol_version.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/libcockatrice/protocol/protocol_version.h" @ONLY
)

add_subdirectory(libcockatrice/protocol/pb)

add_library(libcockatrice_protocol STATIC)

set(SOURCES libcockatrice/protocol/debug_pb_message.cpp libcockatrice/protocol/featureset.cpp
            libcockatrice/protocol/get_pb_extension.cpp libcockatrice/protocol/pending_command.cpp
)

set(HEADERS libcockatrice/protocol/debug_pb_message.h libcockatrice/protocol/featureset.h
            libcockatrice/protocol/get_pb_extension.h libcockatrice/protocol/pending_command.h
)

target_sources(libcockatrice_protocol PRIVATE ${SOURCES} ${HEADERS})

add_dependencies(libcockatrice_protocol libcockatrice_protocol_pb)

# Link the actual generated protobuf library
target_link_libraries(libcockatrice_protocol PUBLIC ${QT_CORE_MODULE} libcockatrice_protocol_pb libcockatrice_utility)

# Expose include paths
target_include_directories(
  libcockatrice_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR} # points to the generated headers
)
