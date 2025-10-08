# Top-level wrapper for the protobuf library

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
target_link_libraries(
  libcockatrice_protocol PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_protocol_pb libcockatrice_utility
)

# Expose include paths
target_include_directories(
  libcockatrice_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR} # points to the generated headers
)
