# Top-level wrapper for the protobuf library

add_subdirectory(pb)

add_library(cockatrice_protocol_wrapper STATIC
    # Optionally, you can add a dummy source if needed
)

# Link the actual generated protobuf library
target_link_libraries(cockatrice_protocol_wrapper
    PUBLIC cockatrice_protocol
)

# Expose the pb/ folder so consumers can do #include "pb/header.pb.h"
target_include_directories(cockatrice_protocol_wrapper
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}  # points to the generated headers
)
