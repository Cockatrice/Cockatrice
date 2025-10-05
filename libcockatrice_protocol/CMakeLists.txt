# Top-level wrapper for the protobuf library

add_subdirectory(libcockatrice/protocol/pb)

add_library(libcockatrice_protocol_wrapper STATIC)

# Link the actual generated protobuf library
target_link_libraries(libcockatrice_protocol_wrapper PUBLIC libcockatrice_protocol)

# Expose the pb/ folder so consumers can do #include <libcockatrice/protocol/pb/header.pb.h>
target_include_directories(
  libcockatrice_protocol_wrapper PUBLIC ${CMAKE_CURRENT_BINARY_DIR} # points to the generated headers
)
