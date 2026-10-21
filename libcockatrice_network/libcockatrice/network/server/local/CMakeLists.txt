set(HEADERS local_server.h local_server_interface.h)

set(SOURCES local_server.cpp local_server_interface.cpp)

add_library(libcockatrice_network_server_local STATIC ${HEADERS} ${SOURCES})

add_dependencies(libcockatrice_network_server_local libcockatrice_protocol)

target_include_directories(libcockatrice_network_server_local PUBLIC .)

target_link_libraries(libcockatrice_network_server_local PUBLIC ${COCKATRICE_QT_MODULES} libcockatrice_protocol)

target_precompile_headers(libcockatrice_network_server_local PRIVATE "${CMAKE_SOURCE_DIR}/cmake/pch/qtcore_pch.h")
