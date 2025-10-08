# Top-level wrapper for the protobuf library

add_subdirectory(pb)

add_library(libcockatrice_protocol STATIC)

set(SOURCES
    pending_command.cpp
)

set(HEADERS
    pending_command.h
)

target_sources(libcockatrice_protocol
    PRIVATE
    ${SOURCES}
    ${HEADERS}
)

add_dependencies(libcockatrice_protocol libcockatrice_protocol_pb)

# Link the actual generated protobuf library
target_link_libraries(libcockatrice_protocol PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_protocol_pb)

# Expose include paths
target_include_directories(
    libcockatrice_protocol
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/pb   # generated pb headers
)
