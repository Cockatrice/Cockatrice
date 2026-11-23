set(HEADERS libcockatrice/server/dev/servatrice_in_memory_database_interface.h)

add_library(
  libcockatrice_server_dev STATIC ${HEADERS} libcockatrice/server/dev/servatrice_in_memory_database_interface.cpp
)

target_include_directories(libcockatrice_server_dev PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libcockatrice_server_dev PUBLIC libcockatrice_network_server_remote)

# Dev-only code: linked by the server tests and the servatrice-dev executable, never by the shipped
# servatrice binary.
